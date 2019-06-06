#include "routing.hpp"
using namespace std;


Routing::Routing(lnxinfo_t *links_info){
    lnxbody_t *node = links_info->body;
    while(true){
        if(node==NULL)
            break;

        all_nodes.push_back(node->remote_phys_port);
        node = node->next;
    }

    node = links_info->body;
    while(true){
        if(node==NULL)
            break;
        
        map<int , int> col;
        for(unsigned int i = 0 ; i < all_nodes.size() ; i++){
            if(all_nodes[i] == node->remote_phys_port)
                col[all_nodes[i]] = 1;
            else
                col[all_nodes[i]] = INFINITY;
        }

        distance_table[node->remote_phys_port] = col;
        node = node->next;
    }
    // for(unsigned int i = 0 ; i < all_nodes.size() ; i++){
    //     for(unsigned int j = 0 ; j < all_nodes.size() ; j++){
    //         cout << distance_table[all_nodes[j]][all_nodes[i]] << " ";
    //     }
    //     cout << endl;
    // }
}

void Routing::fill_routing_table(){

}
