#ifndef G_TOP_LEVEL_H
#define G_TOP_LEVEL_H

#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/render/wlr_renderer.h>
#include "../include/g-server.h"

struct g_server;

struct g_toplevel {
    struct wl_list link;

    struct g_server *server;

    struct wlr_xdg_toplevel *xdg_toplevel;
    struct wlr_surface *surface;

    int pos_x, pos_y;
    int width, height;

    bool mapped;
    bool focused;

    struct wl_list popups;

    struct wl_listener destroy_listener;
    struct wl_listener map_listener;
    struct wl_listener unmap_listener;
    struct wl_listener commit_listener;
    struct wl_listener request_move_listener;
    struct wl_listener request_resize_listener;
    struct wl_listener request_maximize_listener;
    struct wl_listener request_fullscreen_listener;
};

struct g_toplevel* g_toplevel_create(struct g_server *server, struct wlr_xdg_toplevel *xdg_toplevel);
void g_toplevel_destroy(struct g_toplevel *toplevel);

void g_toplevel_on_destroy(struct wl_listener *listener, void *data);
void g_toplevel_on_map(struct wl_listener *listener, void *data);
void g_toplevel_on_unmap(struct wl_listener *listener, void *data);
void g_toplevel_on_commit(struct wl_listener *listener, void *data);
void g_toplevel_on_request_move(struct wl_listener *listener, void *data);
void g_toplevel_on_request_resize(struct wl_listener *listener, void *data);
void g_toplevel_on_request_maximize(struct wl_listener *listener, void *data);
void g_toplevel_on_request_fullscreen(struct wl_listener *listener, void *data);

// Contract
void g_toplevel_on_render_pass(struct g_toplevel *toplevel, struct wlr_render_pass *pass);

void g_toplevel_send_frame_done(struct g_toplevel *toplevel, struct timespec *now);

bool g_toplevel_consume_cursor_button_event(
    struct g_toplevel *toplevel, 
    double x, 
    double y, 
    struct wlr_pointer_button_event *event
);

// Utils
struct g_toplevel* g_toplevel_surface_at(
    struct wl_list *toplevels, 
    double x, double y,
    struct wlr_surface **surface,
    double *surface_x, double *surface_y
);

void g_toplevel_set_focused(struct g_toplevel *toplevel);

#endif