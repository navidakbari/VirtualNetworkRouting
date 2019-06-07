#ifndef _ROUTING_
#define _ROUTING_

#include <climits>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "dbg.hpp"

#include "lnxparse.hpp"
#define INFINITY INT_MAX

struct node_physical_info {
  int port;
  std::string phys_ip;
};

struct routing_table_info {
  std::string best_route_ip;
  int cost;
};

class Routing {
private:
  // distance vector row and cols are virtual address;
  std::map<std::string, std::map<std::string, int> > distance_table;
  std::map<std::string, routing_table_info> routing_table;
  std::map<std::string, node_physical_info> nodes_info;
  // std::vector<int> all_nodes;
  node_physical_info info;

  void fill_nodes_info(lnxinfo_t *links_info);

public:
  Routing(lnxinfo_t *links_info);
  void fill_routing_table(lnxinfo_t *links_info);
};

#endif