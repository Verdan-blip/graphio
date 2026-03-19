#ifndef SW_TOPLEVEL_H
#define SW_TOPLEVEL_H

#include <stdbool.h>
#include <wayland-util.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "../protocols/wlr-foreign-toplevel-management-unstable-v1-client-protocol.h"
#include "gtk/gtk.h"

struct sw_switcher;
struct sw_toplevel_widget;

struct sw_toplevel {
    struct wl_list link;

    struct sw_switcher *switcher;

    struct zwlr_foreign_toplevel_handle_v1 *handle;
    char *title;
    char *app_id;
    bool activated;

    struct sw_toplevel_widget *toplevel_widget;
};

void sw_handle_toplevel(
    void *data,
    struct zwlr_foreign_toplevel_manager_v1 *manager,
    struct zwlr_foreign_toplevel_handle_v1 *handle
);

void sw_handle_title(
    void *data,
    struct zwlr_foreign_toplevel_handle_v1 *handle,
    const char *title
);

void sw_handle_app_id(
    void *data,
    struct zwlr_foreign_toplevel_handle_v1 *handle,
    const char *app_id
);

void sw_handle_state(
    void *data,
    struct zwlr_foreign_toplevel_handle_v1 *handle,
    struct wl_array *states
);

void sw_handle_done(
    void *data,
	struct zwlr_foreign_toplevel_handle_v1 *zwlr_foreign_toplevel_handle_v1
);

void sw_handle_closed(
    void *data,
    struct zwlr_foreign_toplevel_handle_v1 *handle
);

void sw_toplevel_activate(struct sw_toplevel *toplevel);

// UI
GtkWidget* sw_toplevel_create_icon_widget(char *app_id);

#endif