#ifndef G_TOPLEVEL_HANDLE_H
#define G_TOPLEVEL_HANDLE_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_output.h>

struct g_toplevel;

struct g_toplevel_handle {
    struct wlr_foreign_toplevel_handle_v1 *wlr_handle;
    struct g_toplevel *toplevel;

    struct wl_listener on_request_activate;
    struct wl_listener on_request_maximize;
    struct wl_listener on_request_minimize;
    struct wl_listener on_request_fullscreen;
};

struct g_toplevel_handle* g_toplevel_handle_create(
    struct wlr_foreign_toplevel_handle_v1 *wlr_handle
);

void g_toplevel_handle_notify_title_changed(
    struct g_toplevel_handle *handle, 
    const char* title
);

void g_toplevel_handle_notify_app_id_changed(
    struct g_toplevel_handle *handle, 
    const char* app_id
);

void g_toplevel_handle_notify_minimized(struct g_toplevel_handle *handle, bool flag);
void g_toplevel_handle_notify_maximized(struct g_toplevel_handle *handle, bool flag);
void g_toplevel_handle_notify_activated(struct g_toplevel_handle *handle, bool flag);
void g_toplevel_handle_notify_fullscreen(struct g_toplevel_handle *handle, bool flag);

void g_toplevel_handle_destroy(struct g_toplevel_handle *handle);

#endif