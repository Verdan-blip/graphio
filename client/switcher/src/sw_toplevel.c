#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <wayland-util.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <wlr/types/wlr_foreign_toplevel_management_v1.h>

#include "wlr-foreign-toplevel-management-unstable-v1-client-protocol.h"

#include "include/sw_toplevel.h"
#include "include/sw_switcher.h"
#include "include/ui/sw_toplevel_widget.h"
#include "include/ui/sw_switcher_widget.h"
#include "include/wayland/sw_wayland_backend.h"

static const struct zwlr_foreign_toplevel_handle_v1_listener toplevel_impl = {
    .title = sw_handle_title,
    .app_id = sw_handle_app_id,
    .output_enter = NULL,
    .output_leave = NULL,
    .state = sw_handle_state,
    .done = sw_handle_done,
    .closed = sw_handle_closed,
};

void sw_handle_toplevel(
    void *data,
    struct zwlr_foreign_toplevel_manager_v1 *manager,
    struct zwlr_foreign_toplevel_handle_v1 *handle
) {
    struct sw_switcher *switcher = data;

    struct sw_toplevel *toplevel = malloc(sizeof(struct sw_toplevel));
    toplevel->title = NULL;
    toplevel->app_id = NULL;

    sw_switcher_add_toplevel(switcher, toplevel);

    toplevel->handle = handle;
    toplevel->switcher = switcher;
    toplevel->activated = false;

    sw_toplevel_widget_init(toplevel);
    
    sw_switcher_widget_on_add_toplevel(
        switcher->switcher_widget, 
        toplevel->toplevel_widget
    );

    zwlr_foreign_toplevel_handle_v1_add_listener(handle, &toplevel_impl, toplevel);
}

void sw_handle_title(
    void *data,
    struct zwlr_foreign_toplevel_handle_v1 *handle,
    const char *title
) {
    struct sw_toplevel *toplevel = data;

    if (toplevel->title) free(toplevel->title);

    toplevel->title = strdup(title ? title : "unknown");

    printf("sw_toplevel: handled title %s\n", title);
}

void sw_handle_app_id(
    void *data,
    struct zwlr_foreign_toplevel_handle_v1 *handle,
    const char *app_id
) {
    struct sw_toplevel *toplevel = data;

    if (toplevel->title) free(toplevel->app_id);
    toplevel->app_id = strdup(app_id ? app_id : "unknown");

    sw_toplevel_widget_load_icon(toplevel->toplevel_widget);
    sw_switcher_widget_redraw(toplevel->switcher->switcher_widget);

    printf("sw_toplevel: handled app_id %s\n", app_id);
}

void sw_handle_state(
    void *data,
    struct zwlr_foreign_toplevel_handle_v1 *handle,
    struct wl_array *states
) {
    struct sw_toplevel *toplevel = data;
    struct sw_switcher *switcher = toplevel->switcher;

    uint32_t *state;
    wl_array_for_each(state, states) {
        if (*state == ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_ACTIVATED) {
            sw_switcher_notify_toplevel_activated(switcher, toplevel);
            break;
        }
    }
}

void sw_handle_done(
    void *data,
	struct zwlr_foreign_toplevel_handle_v1 *zwlr_foreign_toplevel_handle_v1
) {

}

void sw_handle_closed(
    void *data,
    struct zwlr_foreign_toplevel_handle_v1 *handle
) {
    struct sw_toplevel *toplevel = data;
    struct sw_switcher *switcher = toplevel->switcher;

    sw_switcher_remove_toplevel(switcher, toplevel);

    sw_switcher_widget_on_remove_toplevel(
        toplevel->switcher->switcher_widget,
        toplevel->toplevel_widget
    );

    sw_toplevel_widget_destroy(toplevel->toplevel_widget);

    if (toplevel->app_id) free(toplevel->app_id);
    if (toplevel->title) free(toplevel->title);

    zwlr_foreign_toplevel_handle_v1_destroy(toplevel->handle);
    free(toplevel);
}

void sw_toplevel_activate(struct sw_toplevel *toplevel) {
    struct sw_switcher *switcher = toplevel->switcher;
    struct sw_wayland_backend *backend = switcher->wayland_backend;

    zwlr_foreign_toplevel_handle_v1_activate(
        toplevel->handle, 
        switcher->wayland_backend->seat
    );
}
