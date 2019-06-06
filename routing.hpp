#ifndef _ROUTING_
#define _ROUTING_

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <climits>

#include "lnxparse.hpp"
#define INFINITY INT_MAX


struct interface_info{
    int cost;
    std::string virtual_addr;
    int port;
    std::string ip;
};


class Routing{
    private:
        std::map<int , std::map<int , int> > distance_table;
        std::map<std::string , interface_info> routing_table; 
        std::vector<int> all_nodes;

    public:
        Routing(lnxinfo_t *links_info);
        void fill_routing_table();

};


#endif