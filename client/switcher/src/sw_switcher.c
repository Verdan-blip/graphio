#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#include <wayland-client-protocol.h>
#include <wayland-util.h>

#include "include/sw_switcher.h"
#include "include/sw_graph_model.h"
#include "include/sw_scorer.h"
#include "include/sw_toplevel.h"
#include "include/ui/sw_switcher_widget.h"

static const double SCORER_DEFAULT_ALPHA = 0.1;
static const double SCORER_DEFAULT_DECAY_RATE = 0.001;

static long get_current_time_millis() {
    struct timespec ts;
    
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
        return 0;
    }

    return (long) ts.tv_sec * 1000L + (long)ts.tv_nsec / 1000000L;
}

struct sw_switcher* sw_switcher_create() {
    struct sw_switcher *switcher = malloc(sizeof(struct sw_switcher));

    struct sw_scorer_params params = {
        .alpha = SCORER_DEFAULT_ALPHA,
        .decay_rate = SCORER_DEFAULT_DECAY_RATE
    };

    switcher->scorer = sw_scorer_create(params);

    switcher->graph_model = sw_graph_model_create();
    
    return switcher;
}

void sw_switcher_add_toplevel(
    struct sw_switcher *switcher, 
    struct sw_toplevel *toplevel
) {
    long now = get_current_time_millis();

    double initial_score = sw_scorer_reward(
        switcher->scorer,
        0.0,
        SW_SCORER_EVENT_TYPE_OPEN,
        now,
        now
    );

    struct sw_graph_node *node = malloc(sizeof(struct sw_graph_node));
    node->data = toplevel;
    node->score = initial_score;

    sw_graph_model_add(switcher->graph_model, node);

    toplevel->node = node;
}

void sw_switcher_remove_toplevel(
    struct sw_switcher *switcher, 
    struct sw_toplevel *toplevel
) {
    sw_graph_model_remove(switcher->graph_model, toplevel->node);
}

void sw_switcher_set_activated(
    struct sw_switcher *switcher, 
    struct sw_toplevel *toplevel
) {
    sw_toplevel_activate(toplevel);
}

void sw_switcher_notify_toplevel_activated(
    struct sw_switcher *switcher, 
    struct sw_toplevel *toplevel
) {
    if (toplevel == NULL) return;
    
    struct sw_graph_node *toplevel_node = toplevel->node;
    long now = get_current_time_millis();

    double new_score = sw_scorer_reward(
        switcher->scorer, 
        toplevel_node->score, 
        SW_SCORER_EVENT_TYPE_FOCUS, 
        now, 
        toplevel->last_update
    );

    struct sw_graph_node *node;
    sw_graph_model_for_each(node, switcher->graph_model) {
        struct sw_toplevel *it = (struct sw_toplevel *) node->data;

        if (it == toplevel) {
            it->activated = true;
        } else {
            it->activated = false;
        }
        
        double new_score = sw_scorer_apply_decay(
            switcher->scorer, 
            node->score, 
            it->last_update, 
            now
        );
        
        sw_graph_model_update_node_score(switcher->graph_model, node, new_score);
    }

    switcher->current_toplevel = toplevel;

    sw_switcher_widget_redraw(switcher->switcher_widget);
}

void sw_switcher_destroy(struct sw_switcher *switcher) {
    sw_scorer_destroy(switcher->scorer);
    sw_graph_model_destroy(switcher->graph_model);
    free(switcher);
}