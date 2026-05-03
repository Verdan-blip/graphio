#include <stdlib.h>

#include "include/graph/sw_graph_node.h"

struct sw_graph_node* sw_graph_node_create(void *data, double initial_score) {
    struct sw_graph_node *node = calloc(1, sizeof(struct sw_graph_node));
    node->data = data;
    node->score = initial_score;

    return node;
}

void sw_graph_node_destroy(struct sw_graph_node *node) {
    free(node);
}