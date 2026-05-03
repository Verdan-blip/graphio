#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "include/graph/sw_graph_model.h"

struct sw_graph_model* sw_graph_model_create() {
    struct sw_graph_model *graph_model = malloc(sizeof(struct sw_graph_model));
    graph_model->slot_head = NULL;
    graph_model->slot_tail = NULL;
    graph_model->size = 0;

    graph_model->north_node = NULL;
    graph_model->west_node = NULL;
    graph_model->south_node = NULL;
    graph_model->east_node = NULL;

    return graph_model;
}

void sw_graph_model_destroy(struct sw_graph_model *model) {
    if (!model) return;

    if (model->west_node)  free(model->west_node);
    if (model->east_node)  free(model->east_node);
    if (model->north_node) free(model->north_node);
    if (model->south_node) free(model->south_node);

    struct sw_graph_node *curr = model->slot_head;
    while (curr) {
        struct sw_graph_node *next = curr->next;
        free(curr);
        curr = next;
    }

    free(model);
}

bool sw_graph_model_node_update_score(
    struct sw_graph_model *model, 
    struct sw_graph_node *node,
    double new_score
) {
    if (!model || !node) return false;

    node->score = new_score;

    if (sw_graph_model_node_is_primary(model, node)) {
        return false;
    }

    struct sw_graph_node **slots[4] = {
        &model->west_node, &model->east_node, 
        &model->north_node, &model->south_node
    };

    int weakest_idx = 0;
    double min_score = (*slots[0])->score;

    for (int i = 1; i < 4; i++) {
        if (*slots[i] == NULL || (*slots[i])->score < min_score) {
            weakest_idx = i;
            min_score = (*slots[i]) ? (*slots[i])->score : -1.0;
        }
    }

    if (node->score > min_score) {
        struct sw_graph_node *outgoing = *slots[weakest_idx];

        if (node->prev) node->prev->next = node->next;
        if (node->next) node->next->prev = node->prev;
        
        if (model->slot_head == node) model->slot_head = node->next;
        if (model->slot_tail == node) model->slot_tail = node->prev;

        *slots[weakest_idx] = node;
        node->next = NULL;
        node->prev = NULL;

        if (outgoing != NULL) {
            outgoing->next = model->slot_head;
            outgoing->prev = NULL;
            if (model->slot_head) {
                model->slot_head->prev = outgoing;
            }
            model->slot_head = outgoing;
            if (model->slot_tail == NULL) {
                model->slot_tail = outgoing;
            }
        }

        return true;
    }

    return false;
}

bool sw_graph_model_node_add(
    struct sw_graph_model *model, 
    struct sw_graph_node *node
) {
    if (!model || !node) return false;

    bool added_to_primary = true;

    if (model->west_node == NULL) {
        model->west_node = node;
    } else if (model->east_node == NULL) {
        model->east_node = node;
    } else if (model->north_node == NULL) {
        model->north_node = node;
    } else if (model->south_node == NULL) {
        model->south_node = node;
    } else {
        added_to_primary = false;
        
        node->next = model->slot_head;
        node->prev = NULL;
        
        if (model->slot_head != NULL) {
            model->slot_head->prev = node;
        }
        model->slot_head = node;
        
        if (model->slot_tail == NULL) {
            model->slot_tail = node;
        }
    }

    model->size++;

    if (added_to_primary) {
        return true;
    }

    return false;
}

bool sw_graph_model_node_remove(
    struct sw_graph_model *model, 
    struct sw_graph_node *node
) {
    if (!model || !node) return false;

    struct sw_graph_node **target_slot = NULL;

    if (model->west_node == node) target_slot = &model->west_node;
    else if (model->east_node == node) target_slot = &model->east_node;
    else if (model->north_node == node) target_slot = &model->north_node;
    else if (model->south_node == node) target_slot = &model->south_node;

    bool topology_changed = false;

    if (target_slot != NULL) {
        *target_slot = model->slot_head;
        
        if (model->slot_head != NULL) {
            struct sw_graph_node *new_primary = model->slot_head;
            model->slot_head = new_primary->next;
            
            if (model->slot_head != NULL) {
                model->slot_head->prev = NULL;
            } else {
                model->slot_tail = NULL;
            }
            
            new_primary->next = NULL;
            new_primary->prev = NULL;
        }
        
        topology_changed = true;
    } else {
        if (node->prev) node->prev->next = node->next;
        if (node->next) node->next->prev = node->prev;
        
        if (model->slot_head == node) model->slot_head = node->next;
        if (model->slot_tail == node) model->slot_tail = node->prev;

        topology_changed = false;
    }

    model->size--;

    return topology_changed;
}

bool sw_graph_model_node_is_primary(
    struct sw_graph_model *model,
    struct sw_graph_node *node
) {
    return node == model->north_node ||
        node == model->east_node ||
        node == model->south_node ||
        node == model->west_node;
}

bool sw_graph_model_is_empty(struct sw_graph_model *model) {
    return model->size == 0;
}

bool sw_graph_model_slot_nodes_empty(struct sw_graph_model *model) {
    return model->size <= 4;
}

bool sw_graph_model_primary_nodes_empty(struct sw_graph_model *model) {
    return model->size == 0;
}