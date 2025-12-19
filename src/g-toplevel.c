#include <stdint.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/util/log.h>
#include "../include/g-toplevel-interaction.h"
#include "include/g-cursor.h"
#include "include/g-server.h"
#include "../include/g-toplevel.h"

struct g_toplevel* g_toplevel_create(struct g_server *server, struct wlr_xdg_toplevel *xdg_toplevel) {
    struct g_toplevel *toplevel = calloc(1, sizeof(struct g_toplevel));
    toplevel->server = server;

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

    toplevel->request_maximize_listener.notify = g_toplevel_on_request_maximize;
    wl_signal_add(&toplevel->xdg_toplevel->events.request_maximize, &toplevel->request_maximize_listener);

    toplevel->request_fullscreen_listener.notify = g_toplevel_on_request_fullscreen;
    wl_signal_add(&toplevel->xdg_toplevel->events.request_fullscreen, &toplevel->request_fullscreen_listener);

    toplevel->destroy_listener.notify = g_toplevel_on_destroy;
    wl_signal_add(&toplevel->xdg_toplevel->events.destroy, &toplevel->destroy_listener);

    return toplevel;
}

void g_toplevel_destroy(struct g_toplevel *toplevel) {
    wl_list_remove(&toplevel->map_listener.link);
    wl_list_remove(&toplevel->unmap_listener.link);
    wl_list_remove(&toplevel->commit_listener.link);
    wl_list_remove(&toplevel->destroy_listener.link);
    wl_list_remove(&toplevel->link);
    free(toplevel);
}

void g_toplevel_on_destroy(struct wl_listener *listener, void *data) {
    struct g_toplevel *toplevel = wl_container_of(listener, toplevel, destroy_listener);

    wl_list_remove(&toplevel->map_listener.link);
    wl_list_remove(&toplevel->unmap_listener.link);
    wl_list_remove(&toplevel->commit_listener.link);

    wl_list_remove(&toplevel->request_move_listener.link);
    wl_list_remove(&toplevel->request_resize_listener.link);
    wl_list_remove(&toplevel->request_maximize_listener.link);
    wl_list_remove(&toplevel->request_fullscreen_listener.link);

    wl_list_remove(&toplevel->destroy_listener.link);
    wl_list_remove(&toplevel->link);

    free(toplevel);
}

void g_toplevel_on_map(struct wl_listener *listener, void *data) {
    struct g_toplevel *toplevel = wl_container_of(listener, toplevel, map_listener);
    g_toplevel_set_focused(toplevel);

    toplevel->mapped = true;
}

void g_toplevel_on_unmap(struct wl_listener *listener, void *data) {
    struct g_toplevel *toplevel = wl_container_of(listener, toplevel, unmap_listener);
    toplevel->mapped = false;
}

void g_toplevel_on_commit(struct wl_listener *listener, void *data) {
    struct g_toplevel *toplevel = wl_container_of(listener, toplevel, commit_listener);

    if (toplevel->xdg_toplevel->base->initial_commit) {
        wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, 0, 0);
        toplevel->pos_x = 0;
        toplevel->pos_y = 0;
    }

    struct wlr_fbox window_geo_box;
    wlr_surface_get_buffer_source_box(toplevel->surface, &window_geo_box);

    toplevel->width = window_geo_box.width;
    toplevel->height = window_geo_box.height;
}

void g_toplevel_on_request_move(struct wl_listener *listener, void *data) {
    struct g_toplevel *toplevel = wl_container_of(listener, toplevel, request_move_listener);
    struct g_server *server = toplevel->server;

    struct g_toplevel_interaction *interaction = server->toplevel_interaction;

    interaction->grabbed_toplevel = toplevel;

    interaction->grab_pos_x = server->cursor->wlr_cursor->x - toplevel->pos_x;
    interaction->grab_pos_y = server->cursor->wlr_cursor->y - toplevel->pos_y;

    g_cursor_set_cursor_mode_move(server->cursor);
}

void g_toplevel_on_request_resize(struct wl_listener *listener, void *data) {
    struct g_toplevel *toplevel = wl_container_of(listener, toplevel, request_resize_listener);
    struct g_server *server = toplevel->server;

    struct wlr_xdg_toplevel_resize_event *event = data;

    struct g_toplevel_interaction *interaction = server->toplevel_interaction;

    interaction->grabbed_toplevel = toplevel;

    struct wlr_box *client_box = &toplevel->xdg_toplevel->base->geometry;
    uint32_t edges = event->edges;

	double border_x = (toplevel->pos_x + client_box->x) + ((edges & WLR_EDGE_RIGHT) ? client_box->width : 0);
	double border_y = (toplevel->pos_y + client_box->y) + ((edges & WLR_EDGE_BOTTOM) ? client_box->height : 0);

	server->toplevel_interaction->grab_pos_x = server->cursor->wlr_cursor->x - border_x;
	server->toplevel_interaction->grab_pos_y = server->cursor->wlr_cursor->y - border_y;

	interaction->grabbed_client_box = *client_box;
	interaction->grabbed_client_box.x += toplevel->pos_x;
	interaction->grabbed_client_box.y += toplevel->pos_y;

	interaction->resize_edges = edges;

    g_cursor_set_cursor_mode_resize_horizontal(server->cursor);
}

void g_toplevel_on_request_maximize(struct wl_listener *listener, void *data) {
    struct g_toplevel *toplevel = wl_container_of(listener, toplevel, request_maximize_listener);

    if (toplevel->xdg_toplevel->base->initialized) {
        wlr_xdg_surface_schedule_configure(toplevel->xdg_toplevel->base);
    }
}

void g_toplevel_on_request_fullscreen(struct wl_listener *listener, void *data) {
    struct g_toplevel *toplevel = wl_container_of(listener, toplevel, request_maximize_listener);

    if (toplevel->xdg_toplevel->base->initialized) {
        wlr_xdg_surface_schedule_configure(toplevel->xdg_toplevel->base);
    }
}

// Contract
struct g_toplevel_render_data {
    struct g_toplevel *toplevel;
    struct wlr_render_pass *pass;
};

static void g_toplevel_on_each_xdg_surface(
    struct wlr_surface *s, 
    int sx, int sy, 
    void *data
) {
    struct g_toplevel_render_data *render_data = data;
    struct wlr_texture *texture = wlr_surface_get_texture(s);

    if (!texture) {
        wlr_log(WLR_ERROR, "No texture found for toplevel surface, skipping render");
        return;
    }

    const float alpha = 1.0f;

    wlr_render_pass_add_texture(render_data->pass, &(struct wlr_render_texture_options) {
        .texture = texture,
        .alpha = &alpha,
        .src_box = { 0, 0, s->current.width, s->current.height },
        .dst_box = { 
            render_data->toplevel->pos_x, 
            render_data->toplevel->pos_y, 
            s->current.width, 
            s->current.height 
        }
    });
}

// Contract
void g_toplevel_on_render_pass(struct g_toplevel *toplevel, struct wlr_render_pass *pass) {
    wlr_xdg_surface_for_each_surface(
        toplevel->xdg_toplevel->base, 
        g_toplevel_on_each_xdg_surface, 
        &(struct g_toplevel_render_data) {
            .toplevel = toplevel,
            .pass = pass
        }
    );
}

bool g_toplevel_consume_cursor_button_event(
    struct g_toplevel *toplevel, 
    double x, 
    double y, 
    struct wlr_pointer_button_event *event
) {
    if (event->state == WL_POINTER_BUTTON_STATE_PRESSED) {
        if (x >= toplevel->pos_x && 
            x <= toplevel->pos_x + toplevel->width &&
            y >= toplevel->pos_y && 
            y <= toplevel->pos_y + toplevel->height
        ) {
            g_toplevel_set_focused(toplevel);
            return true;
        }
    }

    return false;
}

// Utils
struct g_toplevel* g_toplevel_at(
    struct wl_list *toplevels, 
    double x, double y,
    double *surface_x, double *surface_y
) {
    struct g_toplevel *toplevel;
    wl_list_for_each_reverse(toplevel, toplevels, link) {

        if (!toplevel->mapped) continue;

        double local_x = x - toplevel->pos_x;
        double local_y = y - toplevel->pos_y;

        struct wlr_surface *surface = wlr_surface_surface_at(
            toplevel->surface, 
            local_x, local_y,
            surface_x, surface_y
        );

        if (surface) {
            return toplevel;
        }
    }

    return NULL;
}

void g_toplevel_set_focused(struct g_toplevel *toplevel) {
    if (!toplevel) return;

    struct g_server *server = toplevel->server;
	struct wlr_seat *seat = server->seat->wlr_seat;

	struct wlr_surface *prev_surface = seat->keyboard_state.focused_surface;
	struct wlr_surface *surface = toplevel->xdg_toplevel->base->surface;
    
	if (prev_surface == surface) return;

    if (prev_surface) {
		struct wlr_xdg_toplevel *prev_xdg_toplevel = wlr_xdg_toplevel_try_from_wlr_surface(prev_surface);
        struct g_toplevel *toplevel = wl_container_of(prev_xdg_toplevel, toplevel, xdg_toplevel);
		if (prev_xdg_toplevel) {
			wlr_xdg_toplevel_set_activated(prev_xdg_toplevel, false);
            toplevel->focused = false;
		}
	}

    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
    
    wl_list_remove(&toplevel->link);
    wl_list_insert(&server->toplevels, &toplevel->link);

    wlr_xdg_toplevel_set_activated(toplevel->xdg_toplevel, true);

    if (keyboard != NULL) {
		wlr_seat_keyboard_notify_enter(seat, surface,
			keyboard->keycodes, 
            keyboard->num_keycodes, 
            &keyboard->modifiers
        );
	}

    toplevel->focused = true;
}