#include "routing.hpp"
using namespace std;

Routing::Routing(lnxinfo_t *links_info) {
  // filling self info
  info.port = links_info->local_phys_port;
  info.phys_ip = "localhost";

  fill_nodes_info(links_info);
  fill_adj_mapping(links_info);
  fill_interfaces(links_info);
  // fill_distance_table(links_info);
  // fill_routing_table();

  // for (auto it = distance_table.begin();
  //      !distance_table.empty() && distance_table.end() != it; it++) {
  //   creation_time[it->first] = (long) time(0);
  // }

  sem_init(&dt_sem, 0, 1);
  sem_init(&rt_sem, 0, 1);
}

void Routing::fill_interfaces(lnxinfo_t *links_info) {
  lnxbody_t *node = links_info->body;
  while (node != NULL) {
    interface new_interface;
    new_interface.local = inet_ntoa(node->local_virt_ip);
    new_interface.remote = inet_ntoa(node->remote_virt_ip);
    new_interface.remote_port = (int)node->remote_phys_port;
    interfaces.push_back(new_interface);
    node = node->next;
  }
}

std::vector<interface> Routing::get_interfaces() { return interfaces; }

std::vector<route> Routing::get_routes() {
  // print_routing_table(routing_table);
  // print_distance_table(distance_table);
  vector<route> routes;
  for (auto it = nodes_info.begin();
       !nodes_info.empty() && nodes_info.end() != it; it++) {
    
    route new_route;
    if (it->second.port == info.port) {
      new_route.cost = 0;
      new_route.dst = it->first;
      new_route.loc = it->first;
      routes.push_back(new_route);
      continue;
    }
    if(routing_table.count(it->second.port) == 0)
      continue;

    new_route.cost = routing_table[it->second.port].cost;
    new_route.dst = it->first;
    new_route.loc = adj_mapping[routing_table[it->second.port].best_route_port];

    routes.push_back(new_route);
  }
  return routes;
}

void Routing::fill_nodes_info(lnxinfo_t *links_info) {
  lnxbody_t *node = links_info->body;
  while (node != NULL) {
    node_physical_info node_info;
    node_info.phys_ip = info.phys_ip;
    node_info.port = info.port;
    nodes_info[inet_ntoa(node->local_virt_ip)] = node_info;

    // node_physical_info node_info_dst;
    // node_info_dst.phys_ip = node->remote_phys_host;
    // node_info_dst.port = node->remote_phys_port;
    // nodes_info[inet_ntoa(node->remote_virt_ip)] = node_info_dst;

    node = node->next;
  }
  // print_nodes_map(nodes_info);
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
  // print_distance_table(distance_table);
}

void Routing::fill_routing_table() {
  sem_wait(&rt_sem);
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
  sem_post(&rt_sem);
  // print_routing_table(routing_table);
  // print_creation_time(creation_time);
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
  std::map<int, std::map<int, int>> new_distance_table = distance_table;
  sem_wait(&dt_sem);
  map<int, int> row;
  for (auto it = adj_mapping.begin();
       !adj_mapping.empty() && it != adj_mapping.end(); it++)
    row[it->first] = INFINITY;
  row[from] = 1;
  new_distance_table[from] = row;
  if (routing_table[from].cost <= 1 || routing_table.count(from) < 1) {
    creation_time[from] = (long)time(0);
  }

  for (auto it = taken_routing_table.begin();
       !taken_routing_table.empty() && it != taken_routing_table.end(); it++) {
    if (it->first == info.port || it->second.best_route_port == info.port)
      continue;

    if (!does_dv_have_row(it->first)) {
      // first recognize new rows
      map<int, int> row;
      for (auto it = adj_mapping.begin();
           !adj_mapping.empty() && it != adj_mapping.end(); it++)
        row[it->first] = INFINITY;
      row[from] = EDGE_WEIGHT + it->second.cost;
      new_distance_table[it->first] = row;
      creation_time[it->first] = (long)time(0);
    } else {
      // second update existing rows
      if (new_distance_table[it->first][from] > EDGE_WEIGHT +
      it->second.cost) {
        creation_time[it->first] = (long)time(0);
        new_distance_table[it->first][from] = EDGE_WEIGHT + it->second.cost;
      }
      if (routing_table[it->first].cost >= EDGE_WEIGHT + it->second.cost) {
        creation_time[it->first] = (long)time(0);
      }
    }
  }

  distance_table = new_distance_table;
  sem_post(&dt_sem);
  fill_routing_table();

  // print_routing_table(routing_table);
  // print_creation_time(creation_time);
}

bool Routing::does_dv_have_row(int row_key) {
  return routing_table.count(row_key) != 0;
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
    usleep(500000);
  }
}

void Routing::send_quit_to_adj(Link link) {
  for (auto it = adj_mapping.begin();
       !adj_mapping.empty() && it != adj_mapping.end(); it++) {
    link.send_quit_msg("127.0.0.1", it->first);
  }
}

void Routing::delete_node(int port) {
  sem_wait(&dt_sem);
  distance_table.erase(port);

  for (auto it = distance_table.begin();
       !distance_table.empty() && it != distance_table.end(); it++) {
    it->second.erase(port);
  }
  sem_post(&dt_sem);
  sem_wait(&rt_sem);
  routing_table.erase(port);
  sem_post(&rt_sem);

  std::map<string, node_physical_info> new_nodes_info;
  for (auto it = nodes_info.begin();
       !nodes_info.empty() && it != nodes_info.end(); it++) {
    if (it->second.port != port){
      new_nodes_info[it->first] = it->second;
    }
  }
  nodes_info = new_nodes_info;

  // adj_mapping.erase(port);
}

void Routing::delete_expired_nodes() {
  while (true) {

    for (auto it = creation_time.begin();
         !creation_time.empty() && it != creation_time.end(); it++) {
      if (it->second + 3 < (long)time(0)) {
        creation_time.erase(it->first);
        delete_node(it->first);
        // cerr << "deleting node " << it->first << endl;
        // break;
      }
    }
    sleep(1);
  }
}

std::map<std::string, node_physical_info> Routing::get_nodes_info() {
  return nodes_info;
}

std::map<int, routing_table_info> Routing::get_routing_table() {
  return routing_table;
}

std::map<int, std::string> Routing::get_adj_mapping(){
  return adj_mapping;
}
