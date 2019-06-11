#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <iostream>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include <functional>
#include "link.hpp"
#include "ip.hpp"

#ifdef READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "dbg.hpp"
#include "lnxparse.hpp"
#include "routing.hpp"

using namespace std;

std::thread sending_routing_table_thread;
std::thread recv_routing_table_thread;
std::thread delete_expired_nodes_thread;
Link *link_layer;
Routing *routing;
vector <string> traceroute_host;

void help_cmd(const char *line) {
    (void) line;

    printf("- help, h: Print this list of commands\n"
	   "- interfaces, li: Print information about each interface, one per line\n"
	   "- routes, lr: Print information about the route to each known destination, one per line\n"
	   "- up [integer]: Bring an interface \"up\" (it must be an existing interface, probably one you brought down)\n"
	   "- down [integer]: Bring an interface \"down\"\n"
	   "- send [ip] [protocol] [payload]: sends payload with protocol=protocol to virtual-ip ip\n"
       "- traceroute [ip]: show traceroute\n"
	   "- q: quit this node\n");
}

void interfaces_cmd(const char *line){
    (void) line;
    print_interfaces(routing->get_interfaces());
}

void routes_cmd(const char *line){
    (void) line;
    print_routes(routing->get_routes());
}

void down_cmd(const char *line){
    unsigned interface;

    if (sscanf(line, "down %u", &interface) != 1) {
	dbg(DBG_ERROR, "syntax error (usage: down [interface])\n");
	return;
    }
    routing->down_interface(interface);
}

void up_cmd(const char *line){
    unsigned interface;

    if (sscanf(line, "up %u", &interface) != 1) {
	dbg(DBG_ERROR, "syntax error (usage: up [interface])\n");
	return;
    }
    routing->up_interface(interface);
}

void send_cmd(const char *line){
    char ip_string[INET_ADDRSTRLEN];
    struct in_addr ip_addr;
    uint8_t protocol;
    int num_consumed;
    char *data;

    if (sscanf(line, "send %s %" SCNu8 "%n", ip_string, &protocol, &num_consumed) != 2) {
	dbg(DBG_ERROR, "syntax error (usage: send [ip] [protocol] [payload])\n");
	return;
    }

    if (inet_pton(AF_INET, ip_string, &ip_addr) == 0) {
	dbg(DBG_ERROR, "syntax error (malformed ip address)\n");
	return;
    }

    data = ((char *)line) + num_consumed + 1;

    if (strlen(data) < 1) {
	dbg(DBG_ERROR, "syntax error (payload unspecified)\n");
	return;
    }

    link_layer->send_user_data(ip_string, data, routing, IPPROTO_DATA);
}

void traceroute_cmd(const char *line){
    char ip_string[INET_ADDRSTRLEN];
    struct in_addr ip_addr;
    if (sscanf(line, "traceroute %s", ip_string) != 1) {
	    dbg(DBG_ERROR, "syntax error (usage: traceroute [ip])\n");
	    return;
    }
    
    if (inet_pton(AF_INET, ip_string, &ip_addr) == 0) {
	dbg(DBG_ERROR, "syntax error (malformed ip address)\n");
	return;
    }
    string vaddr = routing->get_adj_mapping()[routing->get_routing_table()[routing->get_nodes_info()[ip_string].port].best_route_port];
    if(vaddr == "")
        dbg(DBG_ERROR, "this ip is not reachable.\n");
    string tracemsg = "Traceroute from " + vaddr + " to " + ip_string;

    traceroute_host.push_back(tracemsg);
    traceroute_host.push_back(vaddr);
    link_layer->send_user_data(ip_string, "traceroute", routing, IPPROTO_TRACEROUTE);

}
struct {
  const char *command;
  void (*handler)(const char *);
} cmd_table[] = {
  {"help", help_cmd},
  {"h", help_cmd},
  {"interfaces", interfaces_cmd},
  {"li", interfaces_cmd},
  {"routes", routes_cmd},
  {"lr", routes_cmd},
  {"down", down_cmd},
  {"up", up_cmd},
  {"send", send_cmd},
  {"traceroute", traceroute_cmd}
};


void quit_msg_handler(std::string data, iphdr header){
    routing->delete_node(header.saddr);
}

void recv_data_handler(std::string data, iphdr header) {
    string lhip = header.lhIP;
    if(!routing->does_interface_up(lhip))
        return ;
    if(header.daddr == (uint32_t) link_layer->get_self_port()){
        std::cout << "------Node received packet!------\n" << endl;
        std::cout << "\t arrived link: \t\t" << link_layer->get_arrived_interface(header.lhaddr, routing) << endl;
        std::cout << "\t source IP: \t\t"  << header.sourceIP  << endl;
        std::cout << "\t destination IP: \t" << header.desIP << endl;
        std::cout << "\t protocol: \t\t" << (int)header.protocol <<endl;
        std::cout << "\t payload length: \t" << data.length() << endl;
        std::cout << "\t payload: \t\t" << data <<endl;
        std::cout << "----------------------------------" << endl;
        return;
    }
    
    link_layer->forwarding(data, header, routing, IPPROTO_DATA);
}

void recv_routing_table_handler(std::string data, iphdr header) {
    string lhip = header.lhIP;
    if(!routing->does_interface_up(lhip))
        return ;
    std::map<int, routing_table_info> routing_table =
        Link::deserialize_routing_table(data);
    routing->update_distance_table((int) header.saddr, routing_table);
}

void recv_nodes_info_handler(std::string data, iphdr header) {
    string lhip = header.lhIP;
    if(!routing->does_interface_up(lhip))
        return ;
    std::map<std::string, node_physical_info> nodes_info = 
        Link::deserialize_nodes_info(data);
    routing->update_nodes_info(nodes_info);
}

void recv_traceroute_msg(std::string data, iphdr header){
    if(header.daddr == (uint32_t) link_layer->get_self_port()){
        traceroute_host.push_back(header.sourceIP);
        if(data == "traceroute finished"){
            cout << traceroute_host[0] << endl;
            for(uint i = 1; i < traceroute_host.size(); i++){
                cout << i << " " << traceroute_host[i] << endl;
            }
            cout << "Traceroute finished in " << traceroute_host.size() - 1 << " hobs" << endl;
            traceroute_host.clear();
        }
        return;
    }
    link_layer->forwarding(data, header, routing, IPPROTO_TRACEROUTE_RESPONSE);
}

void send_traceroute_msg(std::string data, iphdr header){
    string lhip = header.lhIP;
    if(!routing->does_interface_up(lhip))
        return;

    if((header.daddr == (uint32_t) link_layer->get_self_port())){
        link_layer->send_user_data(header.sourceIP,"traceroute finished",routing, IPPROTO_TRACEROUTE_RESPONSE);
    }else{
        link_layer->send_user_data(header.sourceIP,"traceroute",routing, IPPROTO_TRACEROUTE_RESPONSE);
        link_layer->forwarding(data, header, routing, IPPROTO_TRACEROUTE);
    }
}

struct protocol_handler get_handler(void (*f)(std::string,iphdr) , int protocol){
    struct protocol_handler temp;
    temp.protocol_num = protocol;
    temp.handler = f;
    return temp;
}


int main(int argc, char **argv){
    if (argc != 2) {
	dbg(DBG_ERROR, "usage: %s <linkfile>\n", argv[0]);
	return -1;
    }

#ifdef READLINE
    char* line;
    rl_bind_key('\t', rl_complete);
#else
    char line[LINE_MAX];
#endif
    char cmd[LINE_MAX];
    unsigned i;
    int ret;

    lnxinfo_t *links_info = parse_links(argv[1]);
    link_layer = new Link(links_info->local_phys_port);
    routing = new Routing(links_info);

    link_layer->register_handler(get_handler(&recv_data_handler , IPPROTO_DATA));
    link_layer->register_handler(get_handler(&recv_routing_table_handler, IPPROTO_ROUTING_TABLE));
    link_layer->register_handler(get_handler(&recv_nodes_info_handler, IPPROTO_NODES_INFO));
    link_layer->register_handler(get_handler(&quit_msg_handler, IPPROTO_QUIT_MSG));
    link_layer->register_handler(get_handler(&send_traceroute_msg, IPPROTO_TRACEROUTE));
    link_layer->register_handler(get_handler(&recv_traceroute_msg, IPPROTO_TRACEROUTE_RESPONSE));

    std::thread sending_routing_table_thread(&Routing::send_routing_to_adj, routing, *link_layer);
    std::thread recv_routing_table_thread(&Link::recv_data, link_layer);
    std::thread delete_expired_nodes_thread(&Routing::delete_expired_nodes, routing);

    
    while (1) {
#ifdef READLINE
	if (!(line = readline("> "))) break;
#else
	dbg(DBG_ERROR, "> "); (void)fflush(stdout);
	if (!fgets(line, sizeof(line), stdin)) break;
	if (strlen(line) > 0 && line[strlen(line)-1] == '\n')
	    line[strlen(line)-1] = 0;
#endif

	ret = sscanf(line, "%s", cmd);
	if (ret != 1) {
	    if (ret != EOF) help_cmd(line);
	    continue;
	}
	if (!strcmp(cmd, "q")){
        routing->send_quit_to_adj(*link_layer);
        break;
    } 

	for (i=0; i < sizeof(cmd_table) / sizeof(cmd_table[0]); i++){
	    if (!strcmp(cmd, cmd_table[i].command)){
		cmd_table[i].handler(line);
		break;
	    }
	}

	if (i == sizeof(cmd_table) / sizeof(cmd_table[0])){
	    dbg(DBG_ERROR, "error: no valid command specified\n");
	    help_cmd(line);
	    continue;
	}

#ifdef READLINE
	add_history(line);
	free(line);
#endif
    }
    
    sending_routing_table_thread.detach();
    recv_routing_table_thread.detach();
    delete_expired_nodes_thread.detach();

    printf("\nGoodbye!\n\n");
    return 0;
}
