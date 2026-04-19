#include <stdlib.h>

#include "include/sw_graph_model.h"

struct sw_graph_model* sw_graph_model_create() {
    struct sw_graph_model *graph_model = malloc(sizeof(struct sw_graph_model));
    graph_model->head = NULL;
    graph_model->tail = NULL;
    graph_model->size = 0;

    graph_model->north_node = NULL;
    graph_model->west_node = NULL;
    graph_model->south_node = NULL;
    graph_model->east_node = NULL;

    return graph_model;
}

void sw_graph_model_destroy(struct sw_graph_model *model) {
    if (!model) return;

    struct sw_graph_node *curr = model->head;
    struct sw_graph_node *next;

    while (curr) {
        next = curr->next;
        free(curr);
        
        curr = next;
    }

    free(model);
}

static void refresh_zones(struct sw_graph_model *model) {
    struct sw_graph_node *curr = model->head;

    model->west_node = curr;
    model->east_node = (curr) ? curr->next : NULL;
    model->north_node = (curr && curr->next) ? curr->next->next : NULL;
    model->south_node = (curr && curr->next && curr->next->next) ? curr->next->next->next : NULL;
}

static void swap_nodes(
    struct sw_graph_model *model, 
    struct sw_graph_node *a, 
    struct sw_graph_node *b
) {
    struct sw_graph_node *prev_b = b->prev;
    struct sw_graph_node *next_a = a->next;

    if (prev_b) prev_b->next = a;
    else model->head = a;

    if (next_a) next_a->prev = b;
    else model->tail = b;

    a->prev = prev_b;
    a->next = b;
    b->prev = a;
    b->next = next_a;
}

void sw_graph_model_update_node_score(
    struct sw_graph_model *model, 
    struct sw_graph_node *node, 
    double new_score
) {
    node->score = new_score;

    while (node->prev && node->score > node->prev->score) {
        swap_nodes(model, node, node->prev);
    }

    while (node->next && node->score < node->next->score) {
        swap_nodes(model, node->next, node);
    }

    refresh_zones(model);
}

void sw_graph_model_add(struct sw_graph_model *model, struct sw_graph_node *node) {
    if (!model->head) {
        model->head = node;
        model->tail = node;
        node->prev = NULL;
        node->next = NULL;
    } else {
        node->prev = model->tail;
        node->next = NULL;
        model->tail->next = node;
        model->tail = node;
    }
    model->size++;

    sw_graph_model_update_node_score(model, node, node->score);
}

void sw_graph_model_remove(struct sw_graph_model *model, struct sw_graph_node *node) {
    if (!model || !node) return;

    if (node->prev) {
        node->prev->next = node->next;
    } else {
        model->head = node->next;
    }

    if (node->next) {
        node->next->prev = node->prev;
    } else {
        model->tail = node->prev;
    }

    model->size--;

    node->next = NULL;
    node->prev = NULL;

    refresh_zones(model);
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
    return model->head == NULL;
}

bool sw_graph_model_slot_nodes_empty(struct sw_graph_model *model) {
    return model->size <= 4;
}

bool sw_graph_model_primary_nodes_empty(struct sw_graph_model *model) {
    return model->north_node == NULL &&
        model->west_node == NULL &&
        model->east_node == NULL &&
        model->south_node == NULL;
}

struct sw_graph_node* sw_graph_model_get_first_slot(struct sw_graph_model *model) {
    if (model->south_node == NULL) return NULL;

    return model->south_node->next;
}
