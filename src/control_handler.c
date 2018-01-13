/**
 * @control_handler
 * @author  Shad Ullah Khan <shadulla@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * Handler for the control plane.
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/queue.h>
#include <unistd.h>
#include <string.h>

#include "../include/global.h"
#include "../include/network_util.h"
#include "../include/control_header_lib.h"
#include "../include/author.h"
#include "../include/connection_manager.h"

#ifndef PACKET_USING_STRUCT
    #define CNTRL_CONTROL_CODE_OFFSET 0x04
    #define CNTRL_PAYLOAD_LEN_OFFSET 0x06
#endif

void init_response(int sock_index, char* cntrl_payload);
void routing_response(int sock_index, char *cntrl_payload);
void crash_response(int sock_index,char *cntrl_payload);
void update_response(int sock_index, char *cntrl_payload);

/* Linked List for active control connections */
struct ControlConn
{
    int sockfd;
    LIST_ENTRY(ControlConn) next;
}*connection, *conn_temp;

LIST_HEAD(ControlConnsHead, ControlConn) control_conn_list;

int create_control_sock()
{
    int sock;
    struct sockaddr_in control_addr;
    socklen_t addrlen = sizeof(control_addr);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
        ERROR("socket() failed");

    /* Make socket re-usable */
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (int[]){1}, sizeof(int)) < 0)
        ERROR("setsockopt() failed");

    bzero(&control_addr, sizeof(control_addr));

    control_addr.sin_family = AF_INET;
    control_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    control_addr.sin_port = htons(CONTROL_PORT);
    //printf(" port %d \n"  , control_addr.sin_port);

    if(bind(sock, (struct sockaddr *)&control_addr, sizeof(control_addr)) < 0)
        ERROR("bind() failed");

    if(listen(sock, 5) < 0)
        ERROR("listen() failed");

    LIST_INIT(&control_conn_list);

    return sock;
}

int new_control_conn(int sock_index)
{
    int fdaccept, caddr_len;
    struct sockaddr_in remote_controller_addr;

    caddr_len = sizeof(remote_controller_addr);
    fdaccept = accept(sock_index, (struct sockaddr *)&remote_controller_addr, &caddr_len);
    if(fdaccept < 0)
        ERROR("accept() failed");

    /* Insert into list of active control connections */
    connection = malloc(sizeof(struct ControlConn));
    connection->sockfd = fdaccept;
    LIST_INSERT_HEAD(&control_conn_list, connection, next);

    return fdaccept;
}

void remove_control_conn(int sock_index)
{
    LIST_FOREACH(connection, &control_conn_list, next) {
        if(connection->sockfd == sock_index) LIST_REMOVE(connection, next); // this may be unsafe?
        free(connection);
    }

    close(sock_index);
}

bool isControl(int sock_index)
{
    LIST_FOREACH(connection, &control_conn_list, next)
        if(connection->sockfd == sock_index) return TRUE;

    return FALSE;
}


bool control_recv_hook(int sock_index)
{
    char *cntrl_header, *cntrl_payload;
    uint8_t control_code;
    uint16_t payload_len;

    /* Get control header */
    cntrl_header = (char *) malloc(sizeof(char)*CNTRL_HEADER_SIZE);
    bzero(cntrl_header, CNTRL_HEADER_SIZE);

    if(recvALL(sock_index, cntrl_header, CNTRL_HEADER_SIZE) < 0){
        remove_control_conn(sock_index);
        free(cntrl_header);
        return FALSE;
    }

    /* Get control code and payload length from the header */
    #ifdef PACKET_USING_STRUCT
        /** ASSERT(sizeof(struct CONTROL_HEADER) == 8)
          * This is not really necessary with the __packed__ directive supplied during declaration (see control_header_lib.h).
          * If this fails, comment #define PACKET_USING_STRUCT in control_header_lib.h
          */
        BUILD_BUG_ON(sizeof(struct CONTROL_HEADER) != CNTRL_HEADER_SIZE); // This will FAIL during compilation itself; See comment above.

        struct CONTROL_HEADER *header = (struct CONTROL_HEADER *) cntrl_header;
        control_code = header->control_code;
        payload_len = ntohs(header->payload_len);
    #endif
    #ifndef PACKET_USING_STRUCT
        memcpy(&control_code, cntrl_header+CNTRL_CONTROL_CODE_OFFSET, sizeof(control_code));
        memcpy(&payload_len, cntrl_header+CNTRL_PAYLOAD_LEN_OFFSET, sizeof(payload_len));
        payload_len = ntohs(payload_len);
    #endif

    free(cntrl_header);

    /* Get control payload */
    if(payload_len != 0){
        cntrl_payload = (char *) malloc(sizeof(char)*payload_len);
        bzero(cntrl_payload, payload_len);

        if(recvALL(sock_index, cntrl_payload, payload_len) < 0){
            remove_control_conn(sock_index);
            free(cntrl_payload);
            return FALSE;
        }
    }

    /* Triage on control_code */
    switch(control_code){
        case 0: author_response(sock_index);
                break;
        case 1: init_response(sock_index, cntrl_payload);
                break;
        case 2: routing_response(sock_index, cntrl_payload);
                break;
        case 3: update_response(sock_index, cntrl_payload);
                break;
        case 4: crash_response(sock_index, cntrl_payload);
                break;
              /*
            .........
           .......
         ......*/
    }

    if(payload_len != 0) free(cntrl_payload);
    return TRUE;
}
