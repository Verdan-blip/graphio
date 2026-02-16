#include <stdlib.h>
#include <wlr/types/wlr_scene.h>

#include "include/g_server.h"
#include "include/g_layer_surface.h"

void g_layer_on_create(struct wl_listener *listener, void *data) {
    struct g_server *server = wl_container_of(listener, server, new_layer);

    struct wlr_layer_surface_v1 *wlr_layer_surface = data;

    struct g_layer_surface *layer_surface = calloc(1, sizeof(struct g_layer_surface));

    layer_surface->wlr_layer_surface = wlr_layer_surface;

    struct wlr_scene_layer_surface_v1 *scene_surface = wlr_scene_layer_surface_v1_create(
        &server->scene->tree, wlr_layer_surface);
     
    layer_surface->scene_tree = scene_surface->tree;

    layer_surface->map.notify = g_layer_on_map;
    wl_signal_add(&wlr_layer_surface->surface->events.map, &layer_surface->map);

    layer_surface->unmap.notify = g_layer_on_unmap;
    wl_signal_add(&wlr_layer_surface->surface->events.unmap, &layer_surface->unmap);

    layer_surface->destroy.notify = g_layer_on_destroy;
    wl_signal_add(&wlr_layer_surface->surface->events.destroy, &layer_surface->destroy);

    wl_list_insert(&server->layer_surfaces, &layer_surface->link);

    wlr_layer_surface_v1_configure(
        wlr_layer_surface,
        wlr_layer_surface->current.desired_width,
        wlr_layer_surface->current.desired_height
    );
}

void g_layer_on_map(struct wl_listener *listener, void *data) {
    struct g_layer_surface *layer_surface = wl_container_of(listener, layer_surface, map);

    wlr_scene_node_set_enabled(&layer_surface->scene_tree->node, true);
}

void g_layer_on_unmap(struct wl_listener *listener, void *data) {
    struct g_layer_surface *layer_surface = wl_container_of(listener, layer_surface, unmap);

    wlr_scene_node_set_enabled(&layer_surface->scene_tree->node, false
);
}

void g_layer_on_destroy(struct wl_listener *listener, void *data) {
    struct g_layer_surface *layer_surface = wl_container_of(listener, layer_surface, destroy);

    wl_list_remove(&layer_surface->link);
    free(layer_surface);
}