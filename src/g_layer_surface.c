#include <stdbool.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_scene.h>

#include "include/g_server.h"
#include "wlr-layer-shell-unstable-v1-protocol.h"
#include "include/g_layer_surface.h"

#define G_LAYER_DEFAULT_SURFACE_WIDTH 256
#define G_LAYER_DEFAULT_SURFACE_HEIGHT 256

static void g_layer_surface_on_map(struct wl_listener *listener, void *data) {
    struct g_layer_surface *layer_surface = wl_container_of(listener, layer_surface, map);
}

static void g_layer_surface_on_unmap(struct wl_listener *listener, void *data) {
    struct g_layer_surface *layer_surface = wl_container_of(listener, layer_surface, unmap);
}

static void g_layer_surface_on_client_commit(struct wl_listener *listener, void *data) {
    struct g_layer_surface *layer_surface = wl_container_of(listener, layer_surface, client_commit);

}

static bool g_layer_surface_is_fill_max_width(int anchor) {
    return (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT) && 
        (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
}

static bool g_layer_surface_is_fill_max_height(int anchor) {
    return (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP) && 
        (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM);
}

static void g_layer_surface_on_commit(struct wl_listener *listener, void *data) {
    struct g_layer_surface *layer_surface = wl_container_of(listener, layer_surface, commit);
    struct g_server *server = layer_surface->server;

    if (layer_surface->wlr_layer_surface->initialized) {
        int desired_width = layer_surface->wlr_layer_surface->current.desired_width;
        int desired_height = layer_surface->wlr_layer_surface->current.desired_height;
        int anchor = layer_surface->wlr_layer_surface->current.anchor;

        struct wlr_output *current_output = layer_surface->wlr_layer_surface->output;

        if (current_output == NULL) {
            current_output = g_server_get_current_output(server);
        }

        if (g_layer_surface_is_fill_max_width(anchor)) {
            if (current_output) {
                desired_width = current_output->width;
            } else {
                desired_width = G_LAYER_DEFAULT_SURFACE_WIDTH;
            }
        }

        if (g_layer_surface_is_fill_max_height(anchor)) {
            if (current_output) {
                desired_height = current_output->height;
            } else {
                desired_height = G_LAYER_DEFAULT_SURFACE_HEIGHT;
            }
        }

        wlr_layer_surface_v1_configure(
            layer_surface->wlr_layer_surface, 
            desired_width, 
            desired_height
        );
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

    layer_surface->server = server;
    layer_surface->wlr_layer_surface = wlr_layer_surface;

    struct wlr_scene_layer_surface_v1 *scene_surface = wlr_scene_layer_surface_v1_create(
        server->background_tree, wlr_layer_surface
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
