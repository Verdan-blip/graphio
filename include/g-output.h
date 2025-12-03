#ifndef G_OUTPUT_H
#define G_OUTPUT_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_output.h>
#include "../include/g-server.h"

struct g_output {
	struct wl_list link;

    struct g_server *server;
    struct g_dock_panel *panel;

    struct wlr_output *wlr_output;

    struct wl_listener frame_listener;
	struct wl_listener destroy_listener;
    struct wl_listener request_state_listener;
};

struct g_output* g_output_create(
    struct wlr_output *wlr_output,
    struct g_server *server
);

void g_output_on_new_frame(struct wl_listener *listener, void *data);
void g_output_on_request_state(struct wl_listener *listener, void *data);
void g_output_on_destroy(struct wl_listener *listener, void *data);

#endif