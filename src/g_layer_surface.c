#include <stdio.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_scene.h>

#include "include/g_server.h"
#include "include/g_layer_surface.h"

static void g_layer_surface_on_map(struct wl_listener *listener, void *data) {
    struct g_layer_surface *layer_surface = wl_container_of(listener, layer_surface, map);
}

static void g_layer_surface_on_unmap(struct wl_listener *listener, void *data) {
    struct g_layer_surface *layer_surface = wl_container_of(listener, layer_surface, unmap);
}

static void g_layer_surface_on_client_commit(struct wl_listener *listener, void *data) {
    struct g_layer_surface *layer_surface = wl_container_of(listener, layer_surface, client_commit);

}

static void g_layer_surface_on_commit(struct wl_listener *listener, void *data) {
    struct g_layer_surface *layer_surface = wl_container_of(listener, layer_surface, commit);

    if (layer_surface->wlr_layer_surface->initialized) {
        wlr_layer_surface_v1_configure(layer_surface->wlr_layer_surface, 300, 300);
    }
}

static void g_layer_surface_on_destroy(struct wl_listener *listener, void *data) {
    struct g_layer_surface *layer_surface = wl_container_of(listener, layer_surface, destroy);

    wl_list_remove(&layer_surface->map.link);
    wl_list_remove(&layer_surface->unmap.link);
    wl_list_remove(&layer_surface->commit.link);
    wl_list_remove(&layer_surface->client_commit.link);
    wl_list_remove(&layer_surface->destroy.link);

    wl_list_remove(&layer_surface->link);
    free(layer_surface);
}

void g_init_layer_surface(struct g_server *server, struct wlr_layer_surface_v1 *wlr_layer_surface) {
    struct g_layer_surface *layer_surface = malloc(sizeof(struct g_layer_surface));

    layer_surface->wlr_layer_surface = wlr_layer_surface;

    struct wlr_scene_layer_surface_v1 *scene_surface = wlr_scene_layer_surface_v1_create(
        &server->scene->tree, wlr_layer_surface
    );
     
    layer_surface->scene_tree = scene_surface->tree;

    layer_surface->map.notify = g_layer_surface_on_map;
    wl_signal_add(&wlr_layer_surface->surface->events.map, &layer_surface->map);

    layer_surface->unmap.notify = g_layer_surface_on_unmap;
    wl_signal_add(&wlr_layer_surface->surface->events.unmap, &layer_surface->unmap);

    layer_surface->commit.notify = g_layer_surface_on_commit;
    wl_signal_add(&wlr_layer_surface->surface->events.commit, &layer_surface->commit);

    layer_surface->client_commit.notify = g_layer_surface_on_client_commit;
    wl_signal_add(&wlr_layer_surface->surface->events.client_commit, &layer_surface->client_commit);

    layer_surface->destroy.notify = g_layer_surface_on_destroy;
    wl_signal_add(&wlr_layer_surface->surface->events.destroy, &layer_surface->destroy);

    wl_list_insert(&server->layer_surfaces, &layer_surface->link);
}
