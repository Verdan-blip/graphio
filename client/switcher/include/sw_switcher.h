#ifndef SW_SWITCHER_H
#define SW_SWITCHER_H

#include <stdbool.h>
#include <wayland-util.h>

struct sw_toplevel;
struct sw_switcher_widget;
struct sw_scorer;
struct sw_graph_model;
struct sw_wayland_backend;
struct sw_app_storage;

struct sw_switcher {
    struct sw_wayland_backend *wayland_backend;

    struct sw_toplevel *current_toplevel;
    struct sw_switcher_widget *switcher_widget;

    struct sw_scorer *scorer;
    struct sw_graph_model *graph_model;
    struct sw_app_storage *storage;
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