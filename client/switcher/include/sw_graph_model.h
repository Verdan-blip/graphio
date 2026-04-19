#ifndef SW_GRAPH_MODEL
#define SW_GRAPH_MODEL

#include <stdbool.h>

#define sw_graph_model_for_each(pos, model) \
    for (pos = (model)->head; pos != NULL; pos = pos->next)

#define sw_graph_model_for_each_primary(pos, i, model) \
    for (i = 0, pos = (model)->head; \
         pos != NULL && i < 4; \
         pos = pos->next, i++)

#define sw_graph_model_for_each_primary_unsafe(pos, i, model) \
    for (i = 0, pos = (model)->head; \
         i < 4; \
         (pos = (pos) ? pos->next : NULL), i++)

#define sw_graph_model_for_each_slot(pos, model) \
    for (pos = (model)->head ? (model)->head : NULL, \
         ({ int _i; for(_i = 0; _i < 4 && pos; _i++) pos = pos->next; }); \
         pos != NULL; \
         pos = pos->next)

struct sw_graph_node {
    void *data;
    double score;
    
    struct sw_graph_node *next;
    struct sw_graph_node *prev;
};

struct sw_graph_model {
    struct sw_graph_node *head;
    struct sw_graph_node *tail;
    int size;

    struct sw_graph_node *north_node;
    struct sw_graph_node *south_node;
    struct sw_graph_node *west_node;
    struct sw_graph_node *east_node;
};

struct sw_graph_model* sw_graph_model_create();
void sw_graph_model_destroy(struct sw_graph_model *model);

void sw_graph_model_add(
    struct sw_graph_model *model,
    struct sw_graph_node *node
);

void sw_graph_model_remove(
    struct sw_graph_model *model,
    struct sw_graph_node *node
);

void sw_graph_model_update_node_score(
    struct sw_graph_model *model, 
    struct sw_graph_node *node,
    double new_score
);

bool sw_graph_model_node_is_primary(
    struct sw_graph_model *model,
    struct sw_graph_node *node
);

bool sw_graph_model_is_empty(struct sw_graph_model *model);
bool sw_graph_model_slot_nodes_empty(struct sw_graph_model *model);
bool sw_graph_model_primary_nodes_empty(struct sw_graph_model *model);

struct sw_graph_node* sw_graph_model_get_first_slot(struct sw_graph_model *model);

#endif