#include <stdbool.h>
#include <stdlib.h>

#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_scene.h>
#include <wayland-server-core.h>
#include <wayland-util.h>

#include "include/toplevel/g_toplevel_handle.h"
#include "include/toplevel/g_toplevel.h"
#include "include/g_server.h"

#include "wlr/types/wlr_foreign_toplevel_management_v1.h"

static void on_request_activate(struct wl_listener *listener, void *data) {
    struct g_toplevel_handle *handle = wl_container_of(listener, handle, on_request_activate);
    struct g_server *server = handle->toplevel->server;

    wlr_scene_node_set_enabled(&server->foregound_tree->node, false);

    g_toplevel_focus(handle->toplevel);
}

static void on_request_minimize(struct wl_listener *listener, void *data) {

}

static void on_request_maximize(struct wl_listener *listener, void *data) {

}

static void on_request_fullscreen(struct wl_listener *listener, void *data) {

}

struct g_toplevel_handle* g_toplevel_handle_create(
    struct wlr_foreign_toplevel_handle_v1 *wlr_handle
) {
    struct g_toplevel_handle *handle = malloc(sizeof(struct g_toplevel_handle));

    handle->toplevel = NULL;
    handle->wlr_handle = wlr_handle;

    handle->on_request_activate.notify = on_request_activate;
    wl_signal_add(&wlr_handle->events.request_activate, &handle->on_request_activate);

    handle->on_request_fullscreen.notify = on_request_fullscreen;
    wl_signal_add(&wlr_handle->events.request_fullscreen, &handle->on_request_fullscreen);

    handle->on_request_maximize.notify = on_request_maximize;
    wl_signal_add(&wlr_handle->events.request_maximize, &handle->on_request_maximize);

    handle->on_request_minimize.notify = on_request_minimize;
    wl_signal_add(&wlr_handle->events.request_minimize, &handle->on_request_minimize);

    return handle;
}

void g_toplevel_handle_notify_app_id_changed(
    struct g_toplevel_handle *handle, 
    const char* app_id
) {
    wlr_foreign_toplevel_handle_v1_set_app_id(handle->wlr_handle, app_id);
}

void g_toplevel_handle_notify_title_changed(
    struct g_toplevel_handle *handle, 
    const char* title
) {
    wlr_foreign_toplevel_handle_v1_set_title(handle->wlr_handle, title);
}

void g_toplevel_handle_notify_minimized(struct g_toplevel_handle *handle, bool flag) {
    wlr_foreign_toplevel_handle_v1_set_minimized(handle->wlr_handle, flag);
}

void g_toplevel_handle_notify_maximized(struct g_toplevel_handle *handle, bool flag) {
    wlr_foreign_toplevel_handle_v1_set_maximized(handle->wlr_handle, flag);
}

void g_toplevel_handle_notify_activated(struct g_toplevel_handle *handle, bool flag) {
    wlr_foreign_toplevel_handle_v1_set_activated(handle->wlr_handle, flag);
}

void g_toplevel_handle_notify_fullscreen(struct g_toplevel_handle *handle, bool flag) {
    wlr_foreign_toplevel_handle_v1_set_fullscreen(handle->wlr_handle, flag);
}

void g_toplevel_handle_destroy(struct g_toplevel_handle *handle) {
    wl_list_remove(&handle->on_request_activate.link);
    wl_list_remove(&handle->on_request_fullscreen.link);
    wl_list_remove(&handle->on_request_maximize.link);
    wl_list_remove(&handle->on_request_minimize.link);

    wlr_foreign_toplevel_handle_v1_destroy(handle->wlr_handle);
    free(handle);
}