#ifndef SW_GRAPH_MODEL
#define SW_GRAPH_MODEL

#include <stdbool.h>

#define sw_graph_model_for_each_primary_unsafe(pos, i, model) \
    for (i = 0, pos = (model)->west_node; \
         i < 4; \
         i++, pos = (i == 1) ? (model)->east_node : \
                    (i == 2) ? (model)->north_node : \
                    (i == 3) ? (model)->south_node : NULL)

#define sw_graph_model_for_each_slot(pos, model) \
    for (pos = (model)->slot_head; pos != NULL; pos = pos->next)

#define sw_graph_model_for_each(pos, i, model) \
    for (i = 0, pos = (model)->west_node; \
         i < 5; \
         i++, pos = (i == 1) ? (model)->east_node : \
                    (i == 2) ? (model)->north_node : \
                    (i == 3) ? (model)->south_node : \
                    (i == 4) ? (model)->slot_head : \
                    (pos ? pos->next : NULL))

struct sw_graph_node {
    void *data;
    double score;
    
    struct sw_graph_node *next;
    struct sw_graph_node *prev;
};

struct sw_graph_model {
    int size;

    struct sw_graph_node *north_node;
    struct sw_graph_node *south_node;
    struct sw_graph_node *west_node;
    struct sw_graph_node *east_node;

    struct sw_graph_node *slot_head;
    struct sw_graph_node *slot_tail;
};

struct sw_graph_model* sw_graph_model_create();
void sw_graph_model_destroy(struct sw_graph_model *model);

void sw_graph_model_add(
    struct sw_graph_model *model,
    struct sw_graph_node *node,
    bool *topology_changed
);

void sw_graph_model_remove(
    struct sw_graph_model *model,
    struct sw_graph_node *node,
    bool *topology_changed
);

void sw_graph_model_update_node_score(
    struct sw_graph_model *model, 
    struct sw_graph_node *node,
    double new_score,
    bool *topology_changed
);

bool sw_graph_model_node_is_primary(
    struct sw_graph_model *model,
    struct sw_graph_node *node
);

bool sw_graph_model_is_empty(struct sw_graph_model *model);
bool sw_graph_model_slot_nodes_empty(struct sw_graph_model *model);
bool sw_graph_model_primary_nodes_empty(struct sw_graph_model *model);

#endif