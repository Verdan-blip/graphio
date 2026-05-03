#include <sched.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <wayland-client-protocol.h>
#include <wayland-util.h>

#include "include/sw_switcher.h"
#include "include/graph/sw_graph_model.h"
#include "include/graph/sw_graph_node.h"
#include "include/storage/sw_app_storage.h"
#include "include/sw_scorer.h"
#include "include/sw_toplevel.h"
#include "include/ui/sw_switcher_widget.h"
#include "include/util/sw_time_util.h"

static const double SCORER_DEFAULT_ALPHA = 0.1;
static const double SCORER_DEFAULT_DECAY_RATE = 0.001;

static const double INITIAL_SCORE = 0.0;

static const char *APP_STORAGE_FILENAME = "scores.csv";

static void on_topology_changed(struct sw_switcher *switcher) {
    sw_switcher_widget_notify_topology_changed(switcher->switcher_widget);
    sw_app_storage_sync(switcher->storage);
}

static bool apply_decay_to_nodes(
    struct sw_switcher *switcher,
    long now
) {
    struct sw_graph_model *graph = switcher->graph_model;
    struct sw_scorer *scorer = switcher->scorer;

    struct sw_graph_node *node;
    int index;
    bool topology_changed = false;
    sw_graph_model_for_each(node, index, graph) {
        if (node == NULL) continue;

        double previous_score = node->score;

        struct sw_toplevel *toplevel = node->data;
        long last_update = toplevel->last_update;
        
        double new_score = sw_scorer_apply_decay(
            scorer, 
            previous_score, 
            last_update, 
            now
        );

        topology_changed = sw_graph_model_node_update_score(graph, node, new_score);
    }

    return topology_changed;
}

struct sw_switcher* sw_switcher_create() {
    struct sw_switcher *switcher = malloc(sizeof(struct sw_switcher));

    struct sw_scorer_params params = {
        .alpha = SCORER_DEFAULT_ALPHA,
        .decay_rate = SCORER_DEFAULT_DECAY_RATE
    };

    switcher->scorer = sw_scorer_create(params);
    switcher->graph_model = sw_graph_model_create();
    switcher->storage = sw_app_storage_load(APP_STORAGE_FILENAME);
    
    return switcher;
}

void sw_switcher_add_toplevel(
    struct sw_switcher *switcher, 
    struct sw_toplevel *toplevel
) {
    struct sw_app_storage *storage = switcher->storage;
    struct sw_scorer *scorer = switcher->scorer;
    struct sw_graph_model *graph_model = switcher->graph_model;

    const char* app_id = toplevel->app_id;
    const struct sw_app_storage_data *app_data = sw_app_storage_get(storage, app_id);

    long now = get_current_time_millis();

    double previous_score = INITIAL_SCORE;
    uint64_t last_update = now;

    if (app_data) {
        previous_score = app_data->score;
        last_update = app_data->last_update;
    }

    double initial_score = sw_scorer_reward(
        scorer,
        previous_score,
        SW_SCORER_EVENT_TYPE_OPEN,
        now,
        last_update
    );

    struct sw_graph_node *node = sw_graph_node_create(toplevel, initial_score);

    toplevel->node = node;
    toplevel->last_update = now;

    struct sw_app_storage_data new_app_data = (struct sw_app_storage_data) { .score = initial_score, .last_update = now };
    sw_app_storage_set(storage, app_id, &new_app_data);

    bool topology_changed = sw_graph_model_node_add(graph_model, node);
    if (topology_changed) {
        on_topology_changed(switcher);
    }
}

void sw_switcher_remove_toplevel(
    struct sw_switcher *switcher, 
    struct sw_toplevel *toplevel
) {
    bool topology_changed = sw_graph_model_node_remove(switcher->graph_model, toplevel->node);

    if (topology_changed) {
        on_topology_changed(switcher);
    }
}

void sw_switcher_set_activated(
    struct sw_switcher *switcher, 
    struct sw_toplevel *toplevel
) {
    if (toplevel == NULL || switcher == NULL) return;

    struct sw_app_storage *storage = switcher->storage;
    struct sw_scorer *scorer = switcher->scorer;
    struct sw_graph_model *graph = switcher->graph_model;
    struct sw_graph_node *toplevel_node = toplevel->node;

    sw_toplevel_activate(toplevel);

    const char *app_id = toplevel->app_id;

    long now = get_current_time_millis();
    long last_update = toplevel->last_update;
    double previous_score = toplevel_node->score;

    double new_score = sw_scorer_reward(
        scorer, 
        previous_score, 
        SW_SCORER_EVENT_TYPE_FOCUS, 
        now, 
        last_update
    );

    struct sw_app_storage_data new_app_data = (struct sw_app_storage_data) { .score = new_score, .last_update = now };
    sw_app_storage_set(storage, app_id, &new_app_data);

    bool topology_changed = sw_graph_model_node_update_score(graph, toplevel_node, new_score);
    if (topology_changed) {
        on_topology_changed(switcher);
    }

    toplevel->last_update = now;
}

void sw_switcher_notify_toplevel_activated(
    struct sw_switcher *switcher, 
    struct sw_toplevel *toplevel
) {
    if (toplevel == NULL) return;

    struct sw_app_storage *storage = switcher->storage;
    struct sw_scorer *scorer = switcher->scorer;
    struct sw_graph_model *graph = switcher->graph_model;
    struct sw_graph_node *toplevel_node = toplevel->node;

    const char *app_id = toplevel->app_id;
    
    long now = get_current_time_millis();
    long last_update = toplevel->last_update;
    double previous_score = toplevel_node->score;

    double new_score = sw_scorer_reward(
        scorer, 
        previous_score, 
        SW_SCORER_EVENT_TYPE_FOCUS, 
        now, 
        last_update
    );

    toplevel->activated = true;
    switcher->current_toplevel = toplevel;

    sw_switcher_widget_notify_is_current_toplevel_change(
        switcher->switcher_widget, toplevel->toplevel_widget
    );

    bool topology_changed = apply_decay_to_nodes(switcher, now);
    if (topology_changed) {
        on_topology_changed(switcher);
    }
}

void sw_switcher_destroy(struct sw_switcher *switcher) {
    sw_app_storage_sync(switcher->storage);
    sw_app_storage_destroy(switcher->storage);

    sw_scorer_destroy(switcher->scorer);
    sw_graph_model_destroy(switcher->graph_model);
    free(switcher);
}