#ifndef SW_GRAPH_NODE_H
#define SW_GRAPH_NODE_H

struct sw_graph_node {
    void *data;
    double score;
    
    struct sw_graph_node *next;
    struct sw_graph_node *prev;
};

struct sw_graph_node* sw_graph_node_create(void *data, double initial_score);
void sw_graph_node_destroy(struct sw_graph_node *node);

#endif