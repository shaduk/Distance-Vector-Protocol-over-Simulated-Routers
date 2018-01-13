#ifndef TABLES_H_
#define TABLES_H_

struct RouterTable
{
	uint16_t router_id;
	uint16_t router_port;
	uint16_t data_port;
	uint16_t cost;
	uint32_t router_ip;
  LIST_ENTRY(RouterTable) next;
}*router_table;

struct NeighbourTable
{
	uint16_t router_id;
	uint16_t router_port;
	uint32_t router_ip; 
 LIST_ENTRY(NeighbourTable) next;
}*neighbour_table;

struct ForwardingTable
{
	uint16_t destination_id;
	uint16_t padding;
	uint16_t next_hop_id;
	uint16_t cost;
	uint32_t next_hop_ip; 
	uint16_t next_hop_port; 
LIST_ENTRY(ForwardingTable) next;
}* forwarding_table;

LIST_HEAD(RoutingTableHead, RouterTable) router_table_list;
LIST_HEAD(ForwardingTableHead, ForwardingTable) forwarding_table_list;
LIST_HEAD(NeighbourTableHead, NeighbourTable) neighbour_table_list;

#endif
