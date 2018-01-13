/**
 * @connection_manager
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
 * Connection Manager listens for incoming connections/messages from the
 * controller and other routers and calls the desginated handlers.
 */

#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/queue.h>

#include "../include/global.h"
#include "../include/connection_manager.h"
#include "../include/control_handler.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"
#include "../include/tables.h"
#include "../include/author.h"


void main_loop()
{
    int selret, sock_index, fdaccept;
    initDone = 0;
    while(TRUE){
        watch_list = master_list;
        if(initDone == 0)
        {
          selret = select(head_fd+1, &watch_list, NULL, NULL, NULL);
        }
        else
        {
              selret = select(head_fd+1, &watch_list, NULL, NULL, &tv);
              if(selret == 0)
              {
              send_dv_updates(router_socket);
              tv.tv_sec= (long)interval;
              tv.tv_usec = 0.0;
            }
        }

        if(selret < 0)
            ERROR("select failed.");

        /* Loop through file descriptors to check which ones are ready */
        for(sock_index=0; sock_index<=head_fd; sock_index+=1){

            if(FD_ISSET(sock_index, &watch_list)){

                /* control_socket */
                if(sock_index == control_socket){
                    fdaccept = new_control_conn(sock_index);

                    /* Add to watched socket list */
                    FD_SET(fdaccept, &master_list);
                    if(fdaccept > head_fd) head_fd = fdaccept;
                }

                /* router_socket */
                else if(sock_index == router_socket){
                    char buf[512];
                    struct sockaddr_in remaddr;
                    socklen_t addrlen = sizeof(remaddr);
                    int recvlen = recvfrom(router_socket, buf, 512, 0, (struct sockaddr *)&remaddr, &addrlen);
                    /*
                    printf("Update received : ");
                    int i = 0;
                		printf("\nNodes : %u", buf[i] << 8 & 0xff00 | buf[i+1] & 0xff);
                		i += 2;
                		printf("\n UPDATE RECEIVED FROM SOURCE Router Port : %u", buf[i] << 8 & 0xff00 | buf[i+1] & 0xff);
                		i += 2;
                		printf("\nSOURCE Router IP : %u", buf[i] << 24  & 0xff000000 | buf[i+1] << 16& 0xff0000 | buf[i+2] << 8  & 0xff00 | buf[i+3] & 0xff);
                		i += 4;
                		while(i <= 68)
                		{
                			printf("\nRouter ip : %u", buf[i] << 24  & 0xff000000 | buf[i+1] << 16& 0xff0000 | buf[i+2] << 8  & 0xff00 | buf[i+3] & 0xff);
                			i+=4;
                			printf("-------Router Port : %u", buf[i] << 8 & 0xff00 | buf[i+1] & 0xff);
                			i += 2;
                			printf("-------padding  : %u", buf[i] << 8 & 0xff00 | buf[i+1] & 0xff);
                			i+=2;
                			printf("-------router _id  : %u", buf[i] << 8 & 0xff00 | buf[i+1] & 0xff);
                			i+=2;
                			printf("-------cost  : %u", buf[i] << 8 & 0xff00 | buf[i+1] & 0xff);
                			i+=2;
                		} */

                    uint16_t fields = buf[0]<<8 & 0xff00 | buf[1] & 0xff;
                    printf("\nfields %u\n", fields);
                    uint16_t update_from_port = buf[2] << 8 & 0xff00 | buf[3]  & 0xff;
                    uint32_t update_from_ip = buf[4] << 24  & 0xff000000 | buf[5] << 16& 0xff0000 | buf[6] << 8  & 0xff00 | buf[7] & 0xff;
                    uint16_t cost_to_update_ip = 0;
                    uint16_t next_hop = 0;
                    uint16_t next_hop_port = 0;
                    uint32_t next_hop_ip = 0;

                    LIST_FOREACH(router_table, &router_table_list, next)
                    {
                      printf("\nrouter ip %u \n", router_table->router_ip);
                      printf("\n coming from ip %u \n", update_from_ip);
                        if(router_table->router_ip == update_from_ip)
                        {
                          printf("\nBoth are equal \n");
                          printf("\nRouter id %u\n", router_table->router_id);
                          printf("\nCost to update %u\n", router_table->cost);
                        cost_to_update_ip = router_table->cost;
                        next_hop = router_table->router_id;
                        next_hop_ip = router_table->router_ip;
                        next_hop_port = router_table->router_port;
                        }
                    }

                    int offset = 8;
                    for(int i = 0; i < fields; i++)
        						{
        							router_table = malloc(sizeof(struct RouterTable));
        							 uint32_t r_ip = ((buf[offset]<<24) & 0xff000000) | ((buf[offset+1] <<16) & 0xff0000) | ((buf[offset+2] << 8) & 0xff00) | (buf[offset+3] & 0xff);
        							 offset += 4;
                       printf("\nr_ip %u\n", r_ip);
        							 uint16_t r_port= (buf[offset]<<8)& 0xff00 | buf[offset+1] & 0xff;
        							 offset+=2;
                       printf("\nr_port %u\n", r_port);
        							 uint16_t padding= (buf[offset] << 8)  & 0xff00 | buf[offset+1] & 0xff;
        							 offset+=2;
                       uint16_t r_id= (buf[offset] << 8)  & 0xff00 | buf[offset+1] & 0xff;
        							 offset+=2;
                       printf("\nr_id %u\n", r_id);
        							uint16_t cost= (buf[offset] << 8)  & 0xff00 | buf[offset+1] & 0xff;
        							offset+=2;
                      printf("\ncost %u\n", cost);
                      LIST_FOREACH(router_table, &router_table_list, next)
                  		{
                          if(router_table->router_id == r_id)
                          {
                            /*
                            printf("\nrouter id %u \n", router_table->router_id);
                            printf("\n data from neighbour router id %u \n", r_id);
                            printf("\nrouter id cost %u \n", router_table->cost);
                            printf("\n data from neighbour router id %u \n", cost); */
                            if(router_table->cost > cost_to_update_ip + cost)
                            {
                              router_table->cost = cost_to_update_ip + cost;
                              LIST_FOREACH(forwarding_table, &forwarding_table_list, next)
                   					 {
                   						 if(forwarding_table->destination_id == r_id)
                               {
                                 forwarding_table->next_hop_id = next_hop;
                                 forwarding_table->cost = router_table->cost;
                                 forwarding_table->next_hop_port = next_hop_port;
                                 forwarding_table->next_hop_ip = next_hop_ip;
                               }
                   					 }
                            }
                          }
                      }
        					 }
                   /*
                     printf("Update received : THE new forwarding table is ");
                   printf("------------------------------------------ Forwarding Table START ------------------------------------------ ");

                 	LIST_FOREACH(forwarding_table, &forwarding_table_list, next)
                 	{
                 		printf("\n Destination Id %u -------", forwarding_table->destination_id);
                 		printf(" Next Hop ID %u -------", forwarding_table->next_hop_id);
                 		printf(" Padding %u -------", forwarding_table->padding);
                 		printf(" Cost %u -------\n", forwarding_table->cost);
                 	}
                 	printf("------------------------------------------ Forwarding Table DONE ------------------------------------------ ");
                  */
                }

                /* data_socket */
                else if(sock_index == data_socket){
                    //new_data_conn(sock_index);
                }

                /* Existing connection */
                else{
                    if(isControl(sock_index)){
                        if(!control_recv_hook(sock_index)) FD_CLR(sock_index, &master_list);
                    }
                    //else if isData(sock_index);
                    else ERROR("Unknown socket index");
                }
            }
        }
    }
}

void crash_response(int sock_index,char *cntrl_payload)
{
	char *cntrl_response_header;
	cntrl_response_header = create_response_header(sock_index, 4, 0, 0);
	sendALL(sock_index, cntrl_response_header, 8);
	free(cntrl_response_header);
	FD_CLR(sock_index, &master_list);
	exit(0);
}

void init()
{
    control_socket = create_control_sock();

    //router_socket and data_socket will be initialized after INIT from controller

    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);

    /* Register the control socket */
    FD_SET(control_socket, &master_list);
    head_fd = control_socket;

    main_loop();
}
