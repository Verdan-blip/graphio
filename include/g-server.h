#ifndef G_SERVER_H
#define G_SERVER_H

#include <wlr/render/wlr_texture.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xdg_shell.h>
#include "../include/g-cursor.h"
#include "../include/g-seat.h"

struct g_server {
    struct wlr_backend *backend;
    struct wl_display *display;
	struct wlr_renderer *renderer;
	struct wlr_allocator *allocator;
    struct wlr_xdg_shell *xdg_shell;

    struct wlr_output_layout *output_layout;

	struct wl_listener new_output_listener;
    struct wl_listener new_input_listener;
    struct wl_listener new_toplevel_listener;

	struct g_cursor *cursor;
    struct wl_list outputs;
    struct wl_list toplevels;

    struct g_seat *seat;
};

struct g_server* g_server_create();
void g_server_run(struct g_server *server);
void g_server_destroy(struct g_server *server);

void g_server_on_new_output(struct wl_listener *listener, void *data);
void g_server_on_new_input(struct wl_listener *listener, void *data);
void g_server_on_new_toplevel(struct wl_listener *listener, void *data);

#endif