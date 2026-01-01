#ifndef G_POPUP_H
#define G_POPUP_H

#include <stdbool.h>
#include <time.h>
#include <wayland-server-core.h>
#include <wlr/render/wlr_renderer.h>

struct g_server;

struct g_popup {
    struct wl_list link;
    
    struct g_server *server;

    struct wlr_xdg_popup *xdg_popup;
    struct wlr_surface *surface;

    bool mapped;

    int pos_x, pos_y;
    int width, height;

    struct wl_listener map_listener;
    struct wl_listener unmap_listener;
	struct wl_listener commit_listener;
	struct wl_listener destroy_listener;
};

struct g_popup* g_popup_create(struct g_server *server, struct wlr_xdg_popup *xdg_popup);

void g_popup_on_map(struct wl_listener *listener, void *data);
void g_popup_on_unmap(struct wl_listener *listener, void *data);
void g_popup_on_commit(struct wl_listener *listener, void *data);
void g_popup_on_destroy(struct wl_listener *listener, void *data);

// Contract
void g_popup_on_render_pass(struct g_popup *popup, struct wlr_render_pass *pass);

void g_popup_send_frame_done(struct g_popup *popup, struct timespec *now);

//Utils
struct g_popup* g_popup_at(
    struct wl_list *popups, 
    double x, double y,
    double *surface_x, double *surface_y
);

#endif