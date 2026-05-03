#include <stdlib.h>

#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xcursor_manager.h>

#include "include/g_server.h"
#include "include/cursor/g_cursor.h"
#include "include/seat/g_seat.h"

static void process_cursor_move(struct g_server *server) {
	struct g_toplevel *toplevel = server->grabbed_toplevel;

	wlr_scene_node_set_position(&toplevel->scene_tree->node,
		server->cursor->wlr_cursor->x - server->grab_x,
		server->cursor->wlr_cursor->y - server->grab_y);
}

static void process_cursor_resize(struct g_cursor *cursor) {
    struct g_server *server = cursor->server;
	struct g_toplevel *toplevel = server->grabbed_toplevel;

	double border_x = cursor->wlr_cursor->x - server->grab_x;
	double border_y = cursor->wlr_cursor->y - server->grab_y;

	int new_left = server->grab_geobox.x;
	int new_right = server->grab_geobox.x + server->grab_geobox.width;
	int new_top = server->grab_geobox.y;
	int new_bottom = server->grab_geobox.y + server->grab_geobox.height;

	if (server->resize_edges & WLR_EDGE_TOP) {
		new_top = border_y;
		if (new_top >= new_bottom) {
			new_top = new_bottom - 1;
		}
	} else if (server->resize_edges & WLR_EDGE_BOTTOM) {
		new_bottom = border_y;
		if (new_bottom <= new_top) {
			new_bottom = new_top + 1;
		}
	}
	if (server->resize_edges & WLR_EDGE_LEFT) {
		new_left = border_x;
		if (new_left >= new_right) {
			new_left = new_right - 1;
		}
	} else if (server->resize_edges & WLR_EDGE_RIGHT) {
		new_right = border_x;
		if (new_right <= new_left) {
			new_right = new_left + 1;
		}
	}

	struct wlr_box *geo_box = &toplevel->xdg_toplevel->base->geometry;
	wlr_scene_node_set_position(&toplevel->scene_tree->node, new_left - geo_box->x, new_top - geo_box->y);

	int new_width = new_right - new_left;
	int new_height = new_bottom - new_top;
	wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, new_width, new_height);
}

static void process_cursor_motion(struct g_cursor *cursor, uint32_t time) {
    struct g_server *server = cursor->server;

	if (cursor->cursor_mode == G_CURSOR_MOVE) {
		process_cursor_move(server);
		return;
	} else if (cursor->cursor_mode == G_CURSOR_RESIZE) {
		process_cursor_resize(cursor);
		return;
	}

	double sx, sy;

	struct wlr_surface *surface = NULL;
	struct g_toplevel *toplevel = g_toplevel_at(server, server->cursor->wlr_cursor->x, server->cursor->wlr_cursor->y, &surface, &sx, &sy);

	if (!toplevel) {
		wlr_cursor_set_xcursor(cursor->wlr_cursor, cursor->cursor_mgr, "default");
	}

	if (surface) {
		g_seat_pointer_notify_enter(server->seat, surface, sx, sy);
		g_seat_pointer_notify_motion(server->seat, time, sx, sy);
	} else {
		g_seat_pointer_clear_focus(server->seat);
	}
}

static void server_cursor_motion(struct wl_listener *listener, void *data) {
	struct g_cursor *cursor = wl_container_of(listener, cursor, cursor_motion);
	struct wlr_pointer_motion_event *event = data;

	wlr_cursor_move(cursor->wlr_cursor, &event->pointer->base, event->delta_x, event->delta_y);
	process_cursor_motion(cursor, event->time_msec);
}

static void server_cursor_motion_absolute(struct wl_listener *listener, void *data) {
	struct g_cursor *cursor = wl_container_of(listener, cursor, cursor_motion_absolute);
	struct wlr_pointer_motion_absolute_event *event = data;

	wlr_cursor_warp_absolute(cursor->wlr_cursor, &event->pointer->base, event->x, event->y);
	process_cursor_motion(cursor, event->time_msec);
}

static void server_cursor_button(struct wl_listener *listener, void *data) {
	struct g_cursor *cursor = wl_container_of(listener, cursor, cursor_button);
    struct g_server *server = cursor->server;

	struct wlr_pointer_button_event *event = data;

	g_seat_pointer_notify_button(server->seat, event);

	if (event->state == WL_POINTER_BUTTON_STATE_RELEASED) {
		g_cursor_reset_mode(cursor);
		server->grabbed_toplevel = NULL;
	} else {
		double sx, sy;
		struct wlr_surface *surface = NULL;
		struct g_toplevel *toplevel = g_toplevel_at(server, cursor->wlr_cursor->x, cursor->wlr_cursor->y, &surface, &sx, &sy);
		g_toplevel_focus(toplevel);
	}
}

static void server_cursor_axis(struct wl_listener *listener, void *data) {
	struct g_cursor *cursor = wl_container_of(listener, cursor, cursor_axis);
    struct g_server *server = cursor->server;

	struct wlr_pointer_axis_event *event = data;

	g_seat_pointer_notify_axis(server->seat, event);
}

static void server_cursor_frame(struct wl_listener *listener, void *data) {
	struct g_cursor *cursor = wl_container_of(listener, cursor, cursor_frame);
    struct g_server *server = cursor->server;

	g_seat_pointer_notify_frame(server->seat);
}

struct g_cursor* g_cursor_create(struct g_server *server) {
    struct g_cursor *cursor = calloc(1, sizeof(struct g_cursor));

    cursor->server = server;
    cursor->wlr_cursor = wlr_cursor_create();

	wlr_cursor_attach_output_layout(cursor->wlr_cursor, server->output_layout);

	cursor->cursor_mgr = wlr_xcursor_manager_create(NULL, 24);
	cursor->cursor_mode = G_CURSOR_PASSTHROUGH;

	cursor->cursor_motion.notify = server_cursor_motion;
	wl_signal_add(&cursor->wlr_cursor->events.motion, &cursor->cursor_motion);

	cursor->cursor_motion_absolute.notify = server_cursor_motion_absolute;
	wl_signal_add(&cursor->wlr_cursor->events.motion_absolute, &cursor->cursor_motion_absolute);

	cursor->cursor_button.notify = server_cursor_button;
	wl_signal_add(&cursor->wlr_cursor->events.button, &cursor->cursor_button);

	cursor->cursor_axis.notify = server_cursor_axis;
	wl_signal_add(&cursor->wlr_cursor->events.axis, &cursor->cursor_axis);

	cursor->cursor_frame.notify = server_cursor_frame;
	wl_signal_add(&cursor->wlr_cursor->events.frame, &cursor->cursor_frame);

    return cursor;
}

void g_cursor_destroy(struct g_cursor *cursor) {
    wl_list_remove(&cursor->cursor_motion.link);
	wl_list_remove(&cursor->cursor_motion_absolute.link);
	wl_list_remove(&cursor->cursor_button.link);
	wl_list_remove(&cursor->cursor_axis.link);
	wl_list_remove(&cursor->cursor_frame.link);

	wlr_xcursor_manager_destroy(cursor->cursor_mgr);
	wlr_cursor_destroy(cursor->wlr_cursor);
    free(cursor);
}

void g_cursor_reset_mode(struct g_cursor *cursor) {
    cursor->cursor_mode = G_CURSOR_PASSTHROUGH;
}

void g_cursor_attach_input_device(struct g_cursor *cursor, struct wlr_input_device *device) {
    wlr_cursor_attach_input_device(cursor->wlr_cursor, device);
}