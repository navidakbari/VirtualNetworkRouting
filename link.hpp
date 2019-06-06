#ifndef LINK_H
#define LINK_H
#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>

class Link {
private:
  /* data */
  struct sockaddr_in address;
  int sockfd;

public:
  Link(int port);
  int send_data(std::string data, std::string ip, int port);
  int recv_data(std::string ip, int port);
};

#endif