#ifndef _ROUTING_
#define _ROUTING_

#include "dbg.hpp"
#include "link.hpp"
#include <arpa/inet.h>
#include <climits>
#include <ctime>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <semaphore.h>
#include <string>
#include <chrono>
#include <vector>

#include "lnxparse.hpp"
#define INFINITY INT_MAX
#define EDGE_WEIGHT 1

class Link;

struct node_physical_info {
  int port;
  std::string phys_ip;
};

long long mil();
struct routing_table_info {
  int best_route_port;
  int cost;
};

struct interface {
  std::string remote;
  std::string local;
  int remote_port;
  bool up;
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
  std::map<int, long long> creation_time;
  node_physical_info info;
  sem_t dt_sem; // for distance_table
  sem_t rt_sem; // for routing_table

  void fill_nodes_info(lnxinfo_t *links_info);
  void fill_distance_table(lnxinfo_t *links_info);
  void fill_adj_mapping(lnxinfo_t *links_info);
  void fill_interfaces(lnxinfo_t *links_info);
  void fill_routing_table();
  bool does_dv_have_row(int row_key);
  bool does_local_interface_up(std::string local_ip);

public:
  Routing(lnxinfo_t *links_info);
  void send_routing_to_adj(Link link);
  void send_quit_to_adj(Link link);
  void delete_node(int port);
  void
  update_distance_table(int from,
                        std::map<int, routing_table_info> taken_routing_table);
  void
  update_nodes_info(std::map<std::string, node_physical_info> taken_nodes_info);
  void delete_expired_nodes();
  std::map<std::string, node_physical_info> get_nodes_info();
  std::map<int, routing_table_info> get_routing_table();
  std::vector<interface> get_interfaces();
  std::map<int, std::string> get_adj_mapping();
  std::vector<route> get_routes();
  std::string find_interface(int for_port);
  bool does_interface_up(std::string remote_ip);
  void down_interface(unsigned local);
  void up_interface(unsigned local);
};

#endif