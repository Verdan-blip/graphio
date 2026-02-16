#ifndef G_LAYER_SURFACE_H
#define G_LAYER_SURFACE_H

#include <wlr/types/wlr_layer_shell_v1.h>

struct g_layer_surface {
    struct wl_list link;

    struct wlr_layer_surface_v1 *wlr_layer_surface;
    struct wlr_scene_tree *scene_tree;

    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;
};

void g_layer_on_create(struct wl_listener *listener, void *data);

void g_layer_on_map(struct wl_listener *listener, void *data);
void g_layer_on_unmap(struct wl_listener *listener, void *data);
void g_layer_on_destroy(struct wl_listener *listener, void *data);

#endif