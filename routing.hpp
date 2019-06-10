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
#include <ctime>
#include <semaphore.h>

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

struct interface {
  std::string remote;
  std::string local;
  int remote_port;
};

struct route {
  std::string dst;
  std::string loc;
  int cost;
};

class Routing {
private:
  // distance vector row and cols are phys_port;
  std::map<int, std::map<int, int>> distance_table;
  std::map<int, routing_table_info> routing_table;
  std::map<std::string, node_physical_info> nodes_info;
  std::map<int, std::string> adj_mapping;
  std::vector<interface> interfaces;
  std::map<int, long> creation_time;
  node_physical_info info;
  sem_t dt_sem; // for distance_table
  sem_t rt_sem; // for routing_table

  void fill_nodes_info(lnxinfo_t *links_info);
  void fill_distance_table(lnxinfo_t *links_info);
  void fill_adj_mapping(lnxinfo_t *links_info);
  void fill_interfaces(lnxinfo_t *links_info);
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
  void delete_expired_nodes();
  std::map<std::string, node_physical_info> get_nodes_info();
  std::map<int, routing_table_info> get_routing_table();
  std::vector<interface> get_interfaces();
  std::vector<route> get_routes();
};

#endif