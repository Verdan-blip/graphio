#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wayland-server-core.h>
#include <wayland-util.h>

#include "include/g_toplevel.h"
#include "include/g_server.h"
#include "include/g_toplevel_handle.h"
#include "include/events/g-event-manager.h"
#include "include/generated/contract/protobuf/window_events.pb-c.h"

static char *DEFAULT_APP_ID = "unknown";

struct g_toplevel *g_toplevel_at(
	struct g_server *server, double lx, double ly,
	struct wlr_surface **surface, double *sx, double *sy
) {
	struct wlr_scene_node *node = wlr_scene_node_at(
		&server->main_tree->node, 
		lx, ly, 
		sx, sy
	);

	if (node == NULL || node->type != WLR_SCENE_NODE_BUFFER) {
		return NULL;
	}

	struct wlr_scene_buffer *scene_buffer = wlr_scene_buffer_from_node(node);
	struct wlr_scene_surface *scene_surface = wlr_scene_surface_try_from_buffer(scene_buffer);
	
	if (!scene_surface) {
		return NULL;
	}

	*surface = scene_surface->surface;

	struct wlr_scene_tree *tree = node->parent;
	while (tree != NULL && tree->node.data == NULL) {
		tree = tree->node.parent;
	}

	if (tree != NULL) {
		return tree->node.data;
	}

	return NULL;
}

static void begin_interactive(
	struct g_toplevel *toplevel, 
	enum g_cursor_mode mode, 
	uint32_t edges
) {
	struct g_server *server = toplevel->server;

	server->grabbed_toplevel = toplevel;
	server->cursor_mode = mode;

	if (mode == G_CURSOR_MOVE) {
		server->grab_x = server->cursor->x - toplevel->scene_tree->node.x;
		server->grab_y = server->cursor->y - toplevel->scene_tree->node.y;
	} else {
		struct wlr_box *geo_box = &toplevel->xdg_toplevel->base->geometry;

		double border_x = (toplevel->scene_tree->node.x + geo_box->x) +
			((edges & WLR_EDGE_RIGHT) ? geo_box->width : 0);
		double border_y = (toplevel->scene_tree->node.y + geo_box->y) +
			((edges & WLR_EDGE_BOTTOM) ? geo_box->height : 0);

		server->grab_x = server->cursor->x - border_x;
		server->grab_y = server->cursor->y - border_y;

		server->grab_geobox = *geo_box;
		server->grab_geobox.x += toplevel->scene_tree->node.x;
		server->grab_geobox.y += toplevel->scene_tree->node.y;

		server->resize_edges = edges;
	}
}

static void xdg_toplevel_map(struct wl_listener *listener, void *data) {
	struct g_toplevel *toplevel = wl_container_of(listener, toplevel, map);

	wl_list_insert(&toplevel->server->toplevels, &toplevel->link);

	g_toplevel_focus(toplevel);
}

static void xdg_toplevel_unmap(struct wl_listener *listener, void *data) {
	struct g_toplevel *toplevel = wl_container_of(listener, toplevel, unmap);

	if (toplevel == toplevel->server->grabbed_toplevel) {
		reset_cursor_mode(toplevel->server);
	}

	wl_list_remove(&toplevel->link);
}

static void xdg_toplevel_commit(struct wl_listener *listener, void *data) {
	struct g_toplevel *toplevel = wl_container_of(listener, toplevel, commit);

	if (toplevel->xdg_toplevel->base->initial_commit) {
		wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, 0, 0);
	}
}

static void xdg_toplevel_on_destroy(struct wl_listener *listener, void *data) {
	struct g_toplevel *toplevel = wl_container_of(listener, toplevel, destroy);
	struct g_server *server = toplevel->server;

	struct g_window_event event = {
		.toplevel = toplevel,
		.type = WINDOW_EVENT_TYPE__WINDOW_EVENT_TYPE_DESTROY
	};

	if (server->current_toplevel == toplevel) {
		server->current_toplevel = NULL;
	}

	wl_list_remove(&toplevel->map.link);
	wl_list_remove(&toplevel->unmap.link);
	wl_list_remove(&toplevel->commit.link);
	wl_list_remove(&toplevel->destroy.link);
	wl_list_remove(&toplevel->request_move.link);
	wl_list_remove(&toplevel->request_resize.link);
	wl_list_remove(&toplevel->request_maximize.link);
	wl_list_remove(&toplevel->request_fullscreen.link);
	wl_list_remove(&toplevel->set_app_id.link);
	wl_list_remove(&toplevel->set_title.link);

	g_toplevel_handle_destroy(toplevel->handle);
	if (toplevel->app_id) free(toplevel->app_id);

	free(toplevel);
}

static void xdg_toplevel_request_move(struct wl_listener *listener, void *data) {
	struct g_toplevel *toplevel = wl_container_of(listener, toplevel, request_move);

	begin_interactive(toplevel, G_CURSOR_MOVE, 0);
}

static void xdg_toplevel_request_resize(struct wl_listener *listener, void *data) {
	struct wlr_xdg_toplevel_resize_event *event = data;
	struct g_toplevel *toplevel = wl_container_of(listener, toplevel, request_resize);

	begin_interactive(toplevel, G_CURSOR_RESIZE, event->edges);
}

static void xdg_toplevel_request_maximize(struct wl_listener *listener, void *data) {
	struct g_toplevel *toplevel = wl_container_of(listener, toplevel, request_maximize);
	struct g_server *server = toplevel->server;

	if (!toplevel->xdg_toplevel->base->initialized) return;

	struct wlr_box output_box;
	wlr_output_layout_get_box(server->output_layout, NULL, &output_box);

	if (toplevel->maximized) {
		wlr_scene_node_set_position(&toplevel->scene_tree->node, toplevel->restore_box.x, toplevel->restore_box.y);
		wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, toplevel->restore_box.width, toplevel->restore_box.height);

		toplevel->restore_box.x = 0;
		toplevel->restore_box.y = 0;
		toplevel->restore_box.width = 0;
		toplevel->restore_box.height = 0;

		toplevel->maximized = false;
	} else {
		toplevel->restore_box.x = toplevel->scene_tree->node.x;
		toplevel->restore_box.y = toplevel->scene_tree->node.y;
		toplevel->restore_box.width = toplevel->xdg_toplevel->base->current.geometry.width;
		toplevel->restore_box.height = toplevel->xdg_toplevel->base->current.geometry.height;

		wlr_scene_node_set_position(&toplevel->scene_tree->node, output_box.x, output_box.y);
		wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, output_box.width, output_box.height);

		toplevel->maximized = true;
	}

	wlr_xdg_surface_schedule_configure(toplevel->xdg_toplevel->base);
	
	g_toplevel_handle_notify_maximized(toplevel->handle, toplevel->maximized);
}

static void xdg_toplevel_request_fullscreen(struct wl_listener *listener, void *data) {
	struct g_toplevel *toplevel = wl_container_of(listener, toplevel, request_fullscreen);

	if (toplevel->xdg_toplevel->base->initialized) {
		wlr_xdg_surface_schedule_configure(toplevel->xdg_toplevel->base);
	}

	g_toplevel_handle_notify_fullscreen(toplevel->handle, false);
}

static void xdg_toplevel_set_app_id(struct wl_listener *listener, void *data) {
	struct g_toplevel *toplevel = wl_container_of(listener, toplevel, set_app_id);

	struct wlr_xdg_toplevel *xdg_toplevel = toplevel->xdg_toplevel;

	char* prev_app_id = toplevel->app_id;

	// Only when app_id set first time
	if (prev_app_id == NULL) {
		struct g_window_event event = {
			.toplevel = toplevel,
			.type = WINDOW_EVENT_TYPE__WINDOW_EVENT_TYPE_CREATE
		};
	} else {
		free(prev_app_id);
	}

	toplevel->app_id = xdg_toplevel->app_id == NULL ? DEFAULT_APP_ID : strdup(xdg_toplevel->app_id);

	g_toplevel_handle_notify_app_id_changed(toplevel->handle, toplevel->app_id);
}

static void xdg_toplevel_set_title(struct wl_listener *listener, void *data) {
	struct g_toplevel *toplevel = wl_container_of(listener, toplevel, set_title);

	struct wlr_xdg_toplevel *xdg_toplevel = toplevel->xdg_toplevel;

	const char* title = xdg_toplevel->title == NULL ? "" : strdup(xdg_toplevel->title);

	g_toplevel_handle_notify_title_changed(toplevel->handle, title);
}

static uint32_t generate_toplevel_id() {
	struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
        return (uint32_t)time(NULL);
    }
    
    uint32_t hash = (uint32_t)(ts.tv_sec ^ ts.tv_nsec);
    uint32_t fnv_prime = 16777619;
    uint32_t fnv_offset = 2166136261U;
    
    uint32_t final_hash = (fnv_offset ^ (uint32_t)ts.tv_sec) * fnv_prime;
    final_hash = (final_hash ^ (uint32_t)ts.tv_nsec) * fnv_prime;
    
    return final_hash;
}

void g_toplevel_init(struct g_server *server, struct wlr_xdg_toplevel *xdg_toplevel) {
	struct g_toplevel *toplevel = calloc(1, sizeof(*toplevel));
	toplevel->server = server;
	toplevel->xdg_toplevel = xdg_toplevel;

	toplevel->scene_tree = wlr_scene_xdg_surface_create(toplevel->server->main_tree, xdg_toplevel->base);
	toplevel->scene_tree->node.data = toplevel;
	xdg_toplevel->base->data = toplevel->scene_tree;

	toplevel->map.notify = xdg_toplevel_map;
	wl_signal_add(&xdg_toplevel->base->surface->events.map, &toplevel->map);
	toplevel->unmap.notify = xdg_toplevel_unmap;
	wl_signal_add(&xdg_toplevel->base->surface->events.unmap, &toplevel->unmap);
	toplevel->commit.notify = xdg_toplevel_commit;
	wl_signal_add(&xdg_toplevel->base->surface->events.commit, &toplevel->commit);

	toplevel->destroy.notify = xdg_toplevel_on_destroy;
	wl_signal_add(&xdg_toplevel->events.destroy, &toplevel->destroy);

	toplevel->request_move.notify = xdg_toplevel_request_move;
	wl_signal_add(&xdg_toplevel->events.request_move, &toplevel->request_move);
	toplevel->request_resize.notify = xdg_toplevel_request_resize;
	wl_signal_add(&xdg_toplevel->events.request_resize, &toplevel->request_resize);
	toplevel->request_maximize.notify = xdg_toplevel_request_maximize;
	wl_signal_add(&xdg_toplevel->events.request_maximize, &toplevel->request_maximize);
	toplevel->request_fullscreen.notify = xdg_toplevel_request_fullscreen;
	wl_signal_add(&xdg_toplevel->events.request_fullscreen, &toplevel->request_fullscreen);

	toplevel->set_app_id.notify = xdg_toplevel_set_app_id;
	wl_signal_add(&xdg_toplevel->events.set_app_id, &toplevel->set_app_id);
	toplevel->set_title.notify = xdg_toplevel_set_title;
	wl_signal_add(&xdg_toplevel->events.set_title, &toplevel->set_title);

	struct wlr_foreign_toplevel_handle_v1 *wlr_handle = wlr_foreign_toplevel_handle_v1_create(server->toplevel_manager);
	struct g_toplevel_handle *handle = g_toplevel_handle_create(wlr_handle);

	toplevel->handle = handle;
	handle->toplevel = toplevel;

	toplevel->id = generate_toplevel_id();
}

void g_toplevel_focus(struct g_toplevel *toplevel) {
	if (toplevel == NULL) return;

	struct g_server *server = toplevel->server;
	struct wlr_seat *seat = server->seat;

	struct g_toplevel *prev_toplevel = server->current_toplevel;

	struct wlr_surface *current_toplevel_surface = toplevel->xdg_toplevel->base->surface;
	struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);

	if (keyboard != NULL) {
		wlr_seat_keyboard_notify_enter(
			seat, 
			current_toplevel_surface,
			keyboard->keycodes, 
			keyboard->num_keycodes, 
			&keyboard->modifiers
		);
	}

	if (prev_toplevel == toplevel) return;

	if (prev_toplevel) {
		wlr_xdg_toplevel_set_activated(prev_toplevel->xdg_toplevel, false);
		g_toplevel_handle_notify_activated(prev_toplevel->handle, false);
		prev_toplevel->focused = false;
	}

	wlr_scene_node_raise_to_top(&toplevel->scene_tree->node);
	wl_list_remove(&toplevel->link);
	wl_list_insert(&server->toplevels, &toplevel->link);

	wlr_xdg_toplevel_set_activated(toplevel->xdg_toplevel, true);

	g_toplevel_handle_notify_activated(toplevel->handle, true);

	toplevel->focused = true;

	server->current_toplevel = toplevel;

	struct g_window_event event = {
		.toplevel = toplevel,
		.type = WINDOW_EVENT_TYPE__WINDOW_EVENT_TYPE_FOCUS_IN
	};
}
