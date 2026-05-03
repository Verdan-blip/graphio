#ifndef SW_TOPLEVEL_H
#define SW_TOPLEVEL_H

#include <stdbool.h>
#include <wayland-util.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "../protocols/wlr-foreign-toplevel-management-unstable-v1-client-protocol.h"

struct sw_switcher;
struct sw_toplevel_widget;
struct sw_graph_node;

struct sw_toplevel {
    struct zwlr_foreign_toplevel_handle_v1 *handle;

    struct wl_list link;
    struct sw_graph_node *node;

    struct sw_switcher *switcher;

    char *title;
    char *app_id;
    bool activated;

    long last_update;

    struct sw_toplevel_widget *toplevel_widget;
};

void sw_handle_toplevel(
    void *data,
    struct zwlr_foreign_toplevel_manager_v1 *manager,
    struct zwlr_foreign_toplevel_handle_v1 *handle
);

void sw_toplevel_activate(struct sw_toplevel *toplevel);

#endif