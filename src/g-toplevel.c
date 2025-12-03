#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/util/log.h>
#include "../include/g-toplevel.h"

struct g_toplevel* g_toplevel_create(struct g_server *server, struct wlr_xdg_toplevel *xdg_toplevel) {
    struct g_toplevel *toplevel = calloc(1, sizeof(struct g_toplevel));

    toplevel->xdg_toplevel = xdg_toplevel;
    toplevel->mapped = false;
    toplevel->focused = false;

    toplevel->surface = xdg_toplevel->base->surface;

    // Surface
    toplevel->map_listener.notify = g_toplevel_on_map;
    wl_signal_add(&toplevel->xdg_toplevel->base->surface->events.map, &toplevel->map_listener);

    toplevel->unmap_listener.notify = g_toplevel_on_unmap;
    wl_signal_add(&toplevel->xdg_toplevel->base->surface->events.unmap, &toplevel->unmap_listener);

    toplevel->commit_listener.notify = g_toplevel_on_commit;
    wl_signal_add(&toplevel->xdg_toplevel->base->surface->events.commit, &toplevel->commit_listener);

    // Xdg toplevel
    toplevel->request_move_listener.notify = g_toplevel_on_request_move;
    wl_signal_add(&toplevel->xdg_toplevel->events.request_move, &toplevel->request_move_listener);

    toplevel->request_resize_listener.notify = g_toplevel_on_request_resize;
    wl_signal_add(&toplevel->xdg_toplevel->events.request_resize, &toplevel->request_resize_listener);

    toplevel->destroy_listener.notify = g_toplevel_on_destroy;
    wl_signal_add(&toplevel->xdg_toplevel->events.destroy, &toplevel->destroy_listener);

    return toplevel;
}

void g_toplevel_destroy(struct g_toplevel *toplevel) {
    wl_list_remove(&toplevel->map_listener.link);
    wl_list_remove(&toplevel->unmap_listener.link);
    wl_list_remove(&toplevel->commit_listener.link);
    wl_list_remove(&toplevel->destroy_listener.link);
    free(toplevel);
}

void g_toplevel_on_destroy(struct wl_listener *listener, void *data) {

}

void g_toplevel_on_map(struct wl_listener *listener, void *data) {
    struct g_toplevel *toplevel = wl_container_of(listener, toplevel, map_listener);

    wlr_xdg_toplevel_set_activated(toplevel->xdg_toplevel, true);
}

void g_toplevel_on_unmap(struct wl_listener *listener, void *data) {

}

void g_toplevel_on_commit(struct wl_listener *listener, void *data) {
    struct g_toplevel *toplevel = wl_container_of(listener, toplevel, commit_listener);

    if (toplevel->xdg_toplevel->base->initial_commit) {
        wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, 0, 0);
    }
}

void g_toplevel_on_request_move(struct wl_listener *listener, void *data) {

}

void g_toplevel_on_request_resize(struct wl_listener *listener, void *data) {

}

// Contract
void g_toplevel_on_render_pass(struct g_toplevel *toplevel, struct wlr_render_pass *pass) {
    struct wlr_surface *s = toplevel->surface;
    struct wlr_texture *texture = wlr_surface_get_texture(s);

    if (!texture) {
        wlr_log(WLR_ERROR, "No texture found for surface, skipping render");
        return;
    }

    const float alpha = 1.0f;

    wlr_render_pass_add_texture(pass, &(struct wlr_render_texture_options) {
        .texture = texture,
        .alpha = &alpha,
        .src_box = { 0, 0, s->current.width, s->current.height },
        .dst_box = { 100, 100, s->current.width, s->current.height }
    });
}