#include "routing.hpp"
using namespace std;

Routing::Routing(lnxinfo_t *links_info) {
  // filling self info
  info.port = links_info->local_phys_port;
  info.phys_ip = "localhost";

  fill_nodes_info(links_info);
  fill_adj_mapping(links_info);
  fill_distance_table(links_info);
  fill_routing_table();

  lnxbody_t *node = links_info->body;
}

void Routing::fill_nodes_info(lnxinfo_t *links_info) {
  lnxbody_t *node = links_info->body;
  while (node != NULL) {
    node_physical_info node_info;
    node_info.phys_ip = info.phys_ip;
    node_info.port = info.port;
    nodes_info[inet_ntoa(node->local_virt_ip)] = node_info;

    node_physical_info node_info_dst;
    node_info_dst.phys_ip = node->remote_phys_host;
    node_info_dst.port = node->remote_phys_port;
    nodes_info[inet_ntoa(node->remote_virt_ip)] = node_info_dst;

    node = node->next;
  }
  print_nodes_map(nodes_info);
}

void Routing::fill_distance_table(lnxinfo_t *links_info) {
  lnxbody_t *node = links_info->body;
  while (node != NULL) {
    int dst = node->remote_phys_port;
    lnxbody_t *node2 = links_info->body;
    map<int, int> row;
    while (node2 != NULL) {
      if (node2->remote_phys_port == dst)
        row[node2->remote_phys_port] = 1;
      else
        row[node2->remote_phys_port] = INFINITY;
      node2 = node2->next;
    }
    distance_table[dst] = row;
    node = node->next;
  }
  print_distance_table(distance_table);
}

void Routing::fill_routing_table() {
  for (auto it = distance_table.begin();
       !distance_table.empty() && it != distance_table.end(); it++) {
    map<int, int> row = it->second;
    routing_table_info min;
    min.cost = INFINITY;
    for (auto it2 = row.begin(); !row.empty() && it2 != row.end(); it2++) {
      if (min.cost > it2->second) {
        min.cost = it2->second;
        min.best_route_port = it2->first;
      }
    }
    routing_table[it->first] = min;
  }
  print_routing_table(routing_table);
}

void Routing::fill_adj_mapping(lnxinfo_t *links_info) {
  lnxbody_t *node = links_info->body;
  while (node != NULL) {
    adj_mapping[node->remote_phys_port] = inet_ntoa(node->local_virt_ip);
    node = node->next;
  }
}

void Routing::update_distance_table(
    int from, std::map<int, routing_table_info> taken_routing_table) {
  std::map<int, std::map<int, int>> new_distance_table;
  // first recognize new rows
  for (auto it = taken_routing_table.begin();
       !taken_routing_table.empty() && it != taken_routing_table.end() &&
       !does_dv_have_row(it->first);
       it++) {
    map<int, int> row;
    for (auto it = adj_mapping.begin(); it != adj_mapping.end(); it++)
      row[it->first] = INFINITY;
    row[from] = EDGE_WEIGHT + it->second.cost;
    new_distance_table[it->first] = row;
  }
  // second update existing rows
  for (auto it = taken_routing_table.begin();
       !taken_routing_table.empty() && it != taken_routing_table.end() &&
       does_dv_have_row(it->first);
       it++) {
    if (new_distance_table[it->first][from] > EDGE_WEIGHT + it->second.cost)
      new_distance_table[it->first][from] = EDGE_WEIGHT + it->second.cost;
  }

  distance_table = new_distance_table;
  fill_routing_table();
}

bool Routing::does_dv_have_row(int row_key) {
  return distance_table.count(row_key) != 0;
}

void Routing::update_nodes_info(
    std::map<std::string, node_physical_info> taken_nodes_info) {
  for (auto it = taken_nodes_info.begin();
       !taken_nodes_info.empty() && it != taken_nodes_info.end(); it++) {
    nodes_info[it->first] = it->second;
  }
}

void Routing::send_routing_to_adj(Link link) {
  // link->send_data(&routing_table, );
  // cout <<"sending data to adj" << endl;
  // print_routing_table(routing_table);
  while (true) {
    for (auto it = adj_mapping.begin();
         !adj_mapping.empty() && it != adj_mapping.end(); it++) {
      link.send_routing_table(routing_table, "127.0.0.1", it->first);
      link.send_nodes_info(nodes_info, "127.0.0.1", it->first);
    }
    sleep(1);
  }
}

void Routing::send_quit_to_adj(Link link) {
  for (auto it = adj_mapping.begin();
       !adj_mapping.empty() && it != adj_mapping.end(); it++) {
    link.send_quit_msg("127.0.0.1", it->first);
  }
}