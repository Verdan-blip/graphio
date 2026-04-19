#ifndef SW_SWITCHER_H
#define SW_SWITCHER_H

#include "wlr-foreign-toplevel-management-unstable-v1-client-protocol.h"
#include <stdbool.h>
#include <wayland-util.h>

struct sw_toplevel;
struct sw_switcher_widget;
struct sw_scorer;
struct sw_graph_model;

#define INDEX_LEFT 0
#define INDEX_RIGHT 1
#define INDEX_TOP 2
#define INDEX_BOTTOM 3

#define PRIMATY_TOPLEVEL_COUNT 4

struct sw_switcher {
    struct zwlr_foreign_toplevel_manager_v1 *manager;
    struct wl_seat *seat;

    struct sw_toplevel *current_toplevel;
    struct sw_switcher_widget *switcher_widget;

    struct sw_scorer *scorer;
    struct sw_graph_model *graph_model;
};

struct sw_switcher* sw_switcher_create();
void sw_switcher_add_toplevel(struct sw_switcher *switcher, struct sw_toplevel *toplevel);
void sw_switcher_remove_toplevel(struct sw_switcher *switcher, struct sw_toplevel *toplevel);

void sw_switcher_set_activated(struct sw_switcher *switcher, struct sw_toplevel *toplevel);

void sw_switcher_notify_toplevel_activated(
    struct sw_switcher *switcher, 
    struct sw_toplevel *toplevel
);

void sw_switcher_destroy(struct sw_switcher *switcher);

#endif