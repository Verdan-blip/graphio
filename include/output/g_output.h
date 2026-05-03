#ifndef G_OUTPUT_H
#define G_OUTPUT_H

#include <wayland-server-core.h>
#include <wayland-util.h>

struct g_server;

struct g_output {
	struct wl_list link;

	struct g_server *server;

	struct wlr_output *wlr_output;

	struct wl_listener frame;
	struct wl_listener request_state;
	struct wl_listener destroy;
};

struct g_output* g_output_create(
    struct g_server *server,
    struct wlr_output *wlr_output
);

#endif