#ifndef G_LAYER_SURFACE_H
#define G_LAYER_SURFACE_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_layer_shell_v1.h>

#include "wlr-layer-shell-unstable-v1-protocol.h"

struct g_server;

struct g_layer_surface {
    struct wl_list link;

    struct g_server *server;

    enum zwlr_layer_shell_v1_layer layer;

    struct wlr_layer_surface_v1 *wlr_layer_surface;
    struct wlr_scene_tree *scene_tree;

    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener client_commit;
    struct wl_listener commit;
    struct wl_listener destroy;
};

void g_layer_surface_init(struct g_server *server, struct wlr_layer_surface_v1 *wlr_layer_surface);

#endif