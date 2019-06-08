#ifndef LINK_H
#define LINK_H

#include "dbg.hpp"
#include "ip.hpp"
#include "routing.hpp"
#include <arpa/inet.h>
#include <cstdlib>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

struct protocol_handler {
  int protocol_num;
  void (*handler)(std::string, iphdr header);
};

class Link {
private:
  /* data */
  struct sockaddr_in address;
  int sockfd;
  std::vector<protocol_handler> handlers;
  std::string serialize_routing_table(
      std::map<int, struct routing_table_info> routing_table);
  std::string serialize_nodes_info(
      std::map<std::string, struct node_physical_info> nodes_info);
  std::map<int, struct routing_table_info>
  deserialize_routing_table(std::string data);
  std::map<std::string, struct node_physical_info>
  deserialize_nodes_info(std::string data);
  std::vector<std::string> tokenize(const std::string &str, char delim);

public:
  Link(int port);
  int send_data(iphdr header, std::string data, std::string ip, int port);
  void recv_data();
  void
  send_routing_table(std::map<int, struct routing_table_info> routing_table,
                     std::string ip, int port);
  void
  send_nodes_info(std::map<std::string, struct node_physical_info> nodes_info,
                  std::string ip, int port);
  void send_quit_msg(std::string ip, int port);
  void register_handler(protocol_handler handler);
};

#endif