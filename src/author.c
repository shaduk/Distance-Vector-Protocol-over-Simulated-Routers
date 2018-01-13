/**
 * @author
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
 * AUTHOR [Control Code: 0x00]
 */

#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/queue.h>
#include "../include/global.h"
#include "../include/connection_manager.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"
#include "../include/tables.h"

#define AUTHOR_STATEMENT "I, shadulla, have read and understood the course academic integrity policy."

/* Linked List for active control connections */


void routing_response(int sock_index, char *cntrl_payload)
{
		uint16_t payload_len, response_len;
		char *cntrl_response_header, *cntrl_response_payload, *cntrl_response;

		payload_len = 40; // Discount the NULL chararcter
		cntrl_response_payload = (char *) malloc(payload_len);
		memset(cntrl_response_payload,0,sizeof(cntrl_response_payload));

		int offset = 0;
		LIST_FOREACH(forwarding_table, &forwarding_table_list, next)
		{
			uint16_t router_id = htons(forwarding_table->destination_id);
			memcpy(cntrl_response_payload+offset, &router_id, sizeof(router_id));
			offset += 2;
			uint16_t padding = htons(forwarding_table->padding);
			memcpy(cntrl_response_payload+offset, &padding, sizeof(padding));
			offset += 2;
			uint16_t next_hop_id = htons(forwarding_table->next_hop_id);
			memcpy(cntrl_response_payload+offset, &next_hop_id, sizeof(next_hop_id));
			offset += 2;
			uint16_t cost = htons(forwarding_table->cost);
			memcpy(cntrl_response_payload+offset, &cost, sizeof(cost));
			offset += 2;
		}

		cntrl_response_header = create_response_header(sock_index, 2, 0, payload_len);

		response_len = CNTRL_RESP_HEADER_SIZE+payload_len;
		cntrl_response = (char *) malloc(response_len);
		/* Copy Header */
		memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
		free(cntrl_response_header);
		/* Copy Payload */
		memcpy(cntrl_response+CNTRL_RESP_HEADER_SIZE, cntrl_response_payload, payload_len);
		free(cntrl_response_payload);

		sendALL(sock_index, cntrl_response, response_len);

		free(cntrl_response);
}

void send_dv_updates(int udp_socket)
{
		char *cntrl_response;
		int payload_len = 68; // Discount the NULL chararcter
		cntrl_response = (char *) malloc(payload_len);
		memset(cntrl_response,0,sizeof(cntrl_response));
		int offset = 0;

		uint16_t n_nodes = htons(nodes);
		memcpy(cntrl_response+offset, &n_nodes, sizeof(n_nodes));
		offset += 2;

		uint16_t source_port = htons(MY_ROUTER_PORT);
		memcpy(cntrl_response+offset, &source_port, sizeof(source_port));
		offset += 2;

		uint32_t source_ip = htonl(MY_IP_ADDRESS);
		memcpy(cntrl_response+offset, &source_ip, sizeof(source_ip));
		offset += 4;

		LIST_FOREACH(router_table, &router_table_list, next)
		{
			/*
			printf("\nRouter ip : %u", router_table->router_ip);
			printf("-------Router Port : %u", router_table->router_port);
			printf("-------router _id  : %u", router_table->router_id);
			printf("-------cost  : %u", router_table->cost); */
			uint32_t router_ip = htonl(router_table->router_ip);
			memcpy(cntrl_response+offset, &router_ip, sizeof(router_ip));
			offset += 4;
			uint16_t router_port = htons(router_table->router_port);
			memcpy(cntrl_response+offset, &router_port, sizeof(router_port));
			offset += 2;
			uint16_t padding = htons(0);
			memcpy(cntrl_response+offset, &padding, sizeof(padding));
			offset += 2;
			uint16_t router_id = htons(router_table->router_id);
			memcpy(cntrl_response+offset, &router_id, sizeof(router_id));
			offset += 2;
			uint16_t cost = htons(router_table->cost);
			memcpy(cntrl_response+offset, &cost, sizeof(cost));
			offset += 2;
		}
		/*
		int i = 0;
		printf("\nNodes : %u", cntrl_response[i] << 8 & 0xff00 | cntrl_response[i+1] & 0xff);
		i += 2;
		printf("\n SOURCE Router Port : %u", cntrl_response[i] << 8 & 0xff00 | cntrl_response[i+1] & 0xff);
		i += 2;
		printf("\nSOURCE Router IP : %u", cntrl_response[i] << 24  & 0xff000000 | cntrl_response[i+1] << 16 & 0xff0000 | cntrl_response[i+2] << 8  & 0xff00 | cntrl_response[i+3] & 0xff);
		i += 4;
		while(i <= 68)
		{
			printf("\nprinting i %d \n", i);
			printf("\nRouter ip : %u", cntrl_response[i] << 24  & 0xff000000 | cntrl_response[i+1] << 16& 0xff0000 | cntrl_response[i+2] << 8  & 0xff00 | cntrl_response[i+3] & 0xff);
			i+=4;
			printf("-------Router Port : %u", cntrl_response[i] << 8 & 0xff00 | cntrl_response[i+1] & 0xff);
			i += 2;
			printf("-------padding  : %u", cntrl_response[i] << 8 & 0xff00 | cntrl_response[i+1] & 0xff);
			i+=2;
			printf("-------router _id  : %u", cntrl_response[i] << 8 & 0xff00 | cntrl_response[i+1] & 0xff);
			i+=2;
			printf("-------cost  : %u", cntrl_response[i] << 8 & 0xff00 | cntrl_response[i+1] & 0xff);
			i+=2;
		}
   */
		LIST_FOREACH(neighbour_table, &neighbour_table_list, next)
		{
			LIST_FOREACH(forwarding_table, &forwarding_table_list, next)
			{
				if(neighbour_table->router_id == forwarding_table->destination_id)
				{
						struct sockaddr_in dest;
						dest.sin_family=AF_INET;
						dest.sin_port= htons(forwarding_table->next_hop_port);
						dest.sin_addr.s_addr=   htonl(forwarding_table->next_hop_ip);
						sendto(udp_socket, cntrl_response, payload_len, 0, (struct sockaddr*) &dest, sizeof dest);
				}
			}
		}
		free(cntrl_response);
}

/* Below code taken from https://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/udpserver.c */
void establish_udp_server()
{
	printf("Establisghing UDP server");
  struct sockaddr_in serveraddr; /* server's addr */
  int optval; /* flag value for setsockopt */
  /*
   * socket: create the parent socket
   */
  router_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (router_socket < 0)
	{
    printf("ERROR opening socket");
		return;
	}
  /* setsockopt: Handy debugging trick that lets
   * us rerun the server immediately after we kill it;
   * otherwise we have to wait about 20 secs.
   * Eliminates "ERROR on binding: Address already in use" error.
   */
  optval = 1;
  setsockopt(router_socket, SOL_SOCKET, SO_REUSEADDR,
	     (const void *)&optval , sizeof(int));
  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)MY_ROUTER_PORT);
  /*
   * bind: associate the parent socket with a port
   */
  if (bind(router_socket, (struct sockaddr *) &serveraddr,
	   sizeof(serveraddr)) < 0)
		 {
    printf("ERROR on binding");
		return;
	}
	FD_SET(router_socket, &master_list);
	if(router_socket > head_fd)
	head_fd = router_socket;
	printf("UDP SERVER Succsessfully created");
}


void init_response(int sock_index, char *cntrl_payload)
{

						nodes = cntrl_payload[0]<<8 | cntrl_payload[1];
						interval= cntrl_payload[2]<<8 | cntrl_payload[3];
						int offset=4;
						for(int i = 0; i < nodes; i++)
						{
							router_table = malloc(sizeof(struct RouterTable));
							router_table->router_id = (cntrl_payload[offset]<<8) & 0xff00 | cntrl_payload[offset+1]  & 0xff;
							 offset += 2;
							 router_table->router_port= (cntrl_payload[offset]<<8)& 0xff00 | cntrl_payload[offset+1] & 0xff;
							 offset+=2;
							 router_table->data_port= (cntrl_payload[offset] << 8)  & 0xff00 | cntrl_payload[offset+1] & 0xff;
							 offset+=2;
							router_table->cost= (cntrl_payload[offset] << 8)  & 0xff00 | cntrl_payload[offset+1] & 0xff;
							offset+=2;
							router_table->router_ip = (cntrl_payload[offset] << 24)  & 0xff000000 | cntrl_payload[offset+1] << 16 & 0xff0000 | cntrl_payload[offset+2]  << 8 & 0xff00 | cntrl_payload[offset+3] & 0xff;
							offset+=4;
					 		 LIST_INSERT_HEAD(&router_table_list, router_table, next);
					 }
					 /*
					 printf("------------------------------------------ ROUTER INFO START ------------------------------------------ ");
					 LIST_FOREACH(router_table, &router_table_list, next)
					 {
						 printf("\n Router Id %u -------", router_table->router_id);
						 printf(" Router port %u -------", router_table->router_port);
						 printf(" Data Port %u -------", router_table->data_port);
						 printf(" Cost %u -------", router_table->cost);
						 printf(" Router ");
						 printIP(router_table->router_ip);
					 }
					 printf("------------------------------------------ ROUTER INFO DONE ------------------------------------------ ");
					 */
					 LIST_FOREACH(router_table, &router_table_list, next)
					 {
						 forwarding_table = malloc(sizeof(struct ForwardingTable));
						 if(router_table->cost == 65535)
						 {
							 forwarding_table->destination_id = router_table->router_id;
							 forwarding_table->next_hop_id = 65535;
							 forwarding_table->padding = 0;
							 forwarding_table->cost = 65535;
							 forwarding_table->next_hop_ip = router_table->router_ip;
							 forwarding_table->next_hop_port = router_table->router_port;
						 }
						 else
						 {
									 if(router_table->cost == 0)
									 {
											 MY_ROUTER_PORT = router_table->router_port;
											 MY_IP_ADDRESS = router_table->router_ip;
									 }
									 else {
										 			neighbour_table = malloc(sizeof(struct NeighbourTable));
												  neighbour_table->router_id = router_table->router_id;
													neighbour_table->router_ip = router_table->router_ip;
													neighbour_table->router_port = router_table->router_port;
													LIST_INSERT_HEAD(&neighbour_table_list, neighbour_table, next);
										}
									 forwarding_table->destination_id = router_table->router_id;
									 forwarding_table->next_hop_id = router_table->router_id;
									 forwarding_table->padding = 0;
									 forwarding_table->cost = router_table->cost;
									 forwarding_table->next_hop_ip = router_table->router_ip;
									 forwarding_table->next_hop_port = router_table->router_port;
						 }
						 LIST_INSERT_HEAD(&forwarding_table_list, forwarding_table, next);
					 }

					 establish_udp_server();
					 /*
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
					 /* NOW RESPOND TO INIT */
					 tv.tv_sec= (long)interval;
					 tv.tv_usec = 0.0;
					 initDone = 1;
				 	char *cntrl_response_header;
				 	cntrl_response_header = create_response_header(sock_index, 1, 0, 0);
				 	sendALL(sock_index, cntrl_response_header, 8);
					free(cntrl_response_header);
}

void update_response(int sock_index, char *cntrl_payload)
{
		int offset = 0;
		uint16_t router_id = (cntrl_payload[offset]<<8) & 0xff00 | cntrl_payload[offset+1]  & 0xff;
		offset += 2;
		uint16_t cost = (cntrl_payload[offset]<<8) & 0xff00 | cntrl_payload[offset+1]  & 0xff;
		LIST_FOREACH(router_table, &router_table_list, next)
		{
			if(router_table->router_id == router_id)
			{
				router_table->cost = cost;
				LIST_FOREACH(forwarding_table, &forwarding_table_list, next)
				{
					if(router_id == forwarding_table->destination_id)
					{
						if(forwarding_table->cost > cost)
						{
								forwarding_table->cost = cost;
								forwarding_table->next_hop_id = router_id;
								forwarding_table->next_hop_ip = router_table->router_ip;
								forwarding_table->next_hop_port = router_table->router_port;
						}
					}
				}
			}
		}

		char *cntrl_response_header;
		cntrl_response_header = create_response_header(sock_index, 3, 0, 0);
		sendALL(sock_index, cntrl_response_header, 8);
		free(cntrl_response_header);

}

void author_response(int sock_index)
{
	uint16_t payload_len, response_len;
	char *cntrl_response_header, *cntrl_response_payload, *cntrl_response;

	payload_len = sizeof(AUTHOR_STATEMENT)-1; // Discount the NULL chararcter
	cntrl_response_payload = (char *) malloc(payload_len);
	memcpy(cntrl_response_payload, AUTHOR_STATEMENT, payload_len);

	cntrl_response_header = create_response_header(sock_index, 0, 0, payload_len);

	response_len = CNTRL_RESP_HEADER_SIZE+payload_len;
	cntrl_response = (char *) malloc(response_len);
	/* Copy Header */
	memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	free(cntrl_response_header);
	/* Copy Payload */
	memcpy(cntrl_response+CNTRL_RESP_HEADER_SIZE, cntrl_response_payload, payload_len);
	free(cntrl_response_payload);


	/*
	LIST_FOREACH(forwarding_table, &forwarding_table_list, next)
	{
		printf("\n Destination Id %u -------", forwarding_table->destination_id);
		printf(" Next Hop ID %u -------", forwarding_table->next_hop_id);
		printf(" Padding %u -------", forwarding_table->padding);
		printf(" Cost %u -------\n", forwarding_table->cost);
	}
	printf("------------------------------------------ Forwarding Table DONE ------------------------------------------ ");
	*/
	sendALL(sock_index, cntrl_response, response_len);

	free(cntrl_response);
}
