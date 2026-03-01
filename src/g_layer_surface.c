#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_scene.h>

#include "include/g_server.h"
#include "wlr-layer-shell-unstable-v1-protocol.h"
#include "wlr/util/log.h"
#include "include/g_layer_surface.h"

#define G_LAYER_SURFACE_DEFAULT_WIDTH 256
#define G_LAYER_SURFACE_DEFAULT_HEIGHT 256

static void g_layer_surface_on_map(struct wl_listener *listener, void *data) {
    struct g_layer_surface *layer_surface = wl_container_of(listener, layer_surface, map);
}

static void g_layer_surface_on_unmap(struct wl_listener *listener, void *data) {
    struct g_layer_surface *layer_surface = wl_container_of(listener, layer_surface, unmap);
}

static void g_layer_surface_on_client_commit(struct wl_listener *listener, void *data) {
    struct g_layer_surface *layer_surface = wl_container_of(listener, layer_surface, client_commit);

}

static bool g_layer_surface_is_anchored_horizontally(int anchor) {
    return (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT) && 
        (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
}

static bool g_layer_surface_is_anchored_vertically(int anchor) {
    return (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP) && 
        (anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM);
}

static struct wlr_output* g_layer_surface_safe_get_output(struct g_layer_surface *layer_surface) {
    struct wlr_output *output = layer_surface->wlr_layer_surface->output;

    return output == NULL ? g_server_get_current_output(layer_surface->server) : output;
}

static int g_layer_surface_calculate_width(struct g_layer_surface *layer_surface) {
    struct wlr_layer_surface_v1_state state = layer_surface->wlr_layer_surface->current;

    struct wlr_output *output = g_layer_surface_safe_get_output(layer_surface);

    if (state.desired_width > 0) {
        return ((int32_t) state.desired_width) > output->width 
            ? output->width
            : (int32_t) state.desired_width;
    }

    if (g_layer_surface_is_anchored_horizontally(state.anchor)) {
        return output->width;
    }

    return G_LAYER_SURFACE_DEFAULT_WIDTH;
}

static int g_layer_surface_calculate_height(
    struct g_layer_surface *layer_surface
) {
    struct wlr_layer_surface_v1_state state = layer_surface->wlr_layer_surface->current;

    struct wlr_output *output = g_layer_surface_safe_get_output(layer_surface);

    if (state.desired_height > 0) {
        return ((int32_t) state.desired_height) > output->height 
            ? output->height 
            : (int32_t) state.desired_height;
    }

    if (g_layer_surface_is_anchored_vertically(state.anchor)) {
        return output->height;
    }

    return G_LAYER_SURFACE_DEFAULT_HEIGHT;
}

static int g_layer_surface_calculate_pos_x(
    struct g_layer_surface *layer_surface,
    int calculated_width
) {
    int32_t left = layer_surface->wlr_layer_surface->current.margin.left;
    int32_t right = layer_surface->wlr_layer_surface->current.margin.right;

    struct wlr_output *output = g_layer_surface_safe_get_output(layer_surface);

    if (left == -1 && right == -1) {
        return (output->width - calculated_width) / 2;
    } else if (left == -1) {
        return (output->width - calculated_width);
    } else if (right == -1) {
        return 0;
    }

    return 0;
}

static int g_layer_surface_calculate_pos_y(
    struct g_layer_surface *layer_surface,
    int calculated_height
) {
    int32_t top = layer_surface->wlr_layer_surface->current.margin.top;
    int32_t bottom = layer_surface->wlr_layer_surface->current.margin.bottom;

    struct wlr_output *output = g_layer_surface_safe_get_output(layer_surface);

    if (top == -1 && bottom == -1) {
        return (output->height - calculated_height) / 2;
    } else if (top == -1) {
        return (output->height - calculated_height);
    } else if (bottom == -1) {
        return 0;
    }

    return 0;
}

static void g_layer_surface_on_commit(struct wl_listener *listener, void *data) {
    struct g_layer_surface *layer_surface = wl_container_of(listener, layer_surface, commit);
    struct g_server *server = layer_surface->server;

    if (layer_surface->wlr_layer_surface->initialized) {

        int width = g_layer_surface_calculate_width(layer_surface);
        int height = g_layer_surface_calculate_height(layer_surface);

        int pos_x = g_layer_surface_calculate_pos_x(layer_surface, width);
        int pos_y = g_layer_surface_calculate_pos_y(layer_surface, height);

        wlr_layer_surface_v1_configure(
            layer_surface->wlr_layer_surface, 
            width, 
            height
        );

        wlr_scene_node_set_position(&layer_surface->scene_tree->node, pos_x, pos_y);
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

    enum zwlr_layer_shell_v1_layer layer = layer_surface->wlr_layer_surface->current.layer;

    struct wlr_scene_layer_surface_v1 *scene_surface = NULL;

    if (layer == ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND) {
        scene_surface = wlr_scene_layer_surface_v1_create(
            server->background_tree, 
            wlr_layer_surface
        );
    } else if (layer == ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY) {
        scene_surface = wlr_scene_layer_surface_v1_create(
            server->foregound_tree, 
            wlr_layer_surface
        );
    } else {
        wlr_log(WLR_ERROR, "g_layer_surface: unsupported layer type, change to LAYER_BACKGROUND");
        scene_surface = wlr_scene_layer_surface_v1_create(
            server->background_tree, 
            wlr_layer_surface
        );
    }

     
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
