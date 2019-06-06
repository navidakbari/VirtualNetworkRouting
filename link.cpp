#include "link.hpp"
using namespace std;

Link::Link(int port) {
  // Creating socket file descriptor
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  // memset(&address, 0, sizeof(address));

  // Filling server information
  address.sin_family = AF_INET; // IPv4
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  // Bind the socket with the server address
  if (bind(sockfd, (const struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }
}

int Link::send_data(std::string data, std::string ip, int port) {
  struct sockaddr_in client_addr;
  // memset(&client_addr, 0, sizeof(client_addr));

  client_addr.sin_family = AF_INET;
  client_addr.sin_addr.s_addr = inet_addr(ip.c_str());
  client_addr.sin_port = htons(port);

  int size = sendto(sockfd, data.c_str(), data.length(), 0,
                    (const struct sockaddr *)&client_addr, sizeof(client_addr));

  return size;
}

int Link::recv_data(std::string ip, int port) {
  struct sockaddr_in client_addr;
  // memset(&client_addr, 0, sizeof(client_addr));

  client_addr.sin_family = AF_INET;
  client_addr.sin_addr.s_addr = inet_addr(ip.c_str());
  client_addr.sin_port = htons(port);

  char buffer[1400];
  int size;
  size = recvfrom(sockfd, (char *)buffer, 1400,  
                0, (struct sockaddr *) &client_addr, 
                0); 

  cout << buffer << endl;
  return size;
}
