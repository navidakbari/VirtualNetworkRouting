#ifndef _ROUTING_
#define _ROUTING_

#include "dbg.hpp"
#include "link.hpp"
#include <arpa/inet.h>
#include <climits>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <string>
#include <vector>

#include "lnxparse.hpp"
#define INFINITY INT_MAX
#define EDGE_WEIGHT 1

class Link;

struct node_physical_info {
  int port;
  std::string phys_ip;
};

struct routing_table_info {
  int best_route_port;
  int cost;
};

class Routing {
private:
  // distance vector row and cols are phys_port;
  std::map<int, std::map<int, int>> distance_table;
  std::map<int, routing_table_info> routing_table;
  std::map<std::string, node_physical_info> nodes_info;
  std::map<int, std::string> adj_mapping;
  // std::vector<int> all_nodes;
  node_physical_info info;

  void fill_nodes_info(lnxinfo_t *links_info);
  void fill_distance_table(lnxinfo_t *links_info);
  void fill_adj_mapping(lnxinfo_t *links_info);
  void fill_routing_table();
  bool does_dv_have_row(int row_key);

public:
  Routing(lnxinfo_t *links_info);
  void send_routing_to_adj(Link link);
  void send_quit_to_adj(Link link);
  void delete_node(int port);
  void update_distance_table(int from,
                        std::map<int, routing_table_info> taken_routing_table);
  void
  update_nodes_info(std::map<std::string, node_physical_info> taken_nodes_info);
};

#endif