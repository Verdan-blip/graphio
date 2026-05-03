#include <stdlib.h>
#include <wayland-util.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_pointer.h>

#include "include/seat/g_seat.h"
#include "include/g_layer_surface.h"
#include "include/keyboard/g_keyboard.h"
#include "include/g_server.h"

static const char* DEFAULT_SEAT_NAME = "seat0";

static void seat_request_cursor(struct wl_listener *listener, void *data) {
    struct g_seat *seat = wl_container_of(listener, seat, request_cursor);
	struct wlr_seat_pointer_request_set_cursor_event *event = data;
	struct wlr_seat_client *focused_client = seat->wlr_seat->pointer_state.focused_client;

    // Ignoring: graphio allows to use only one cursor

	/* 
    if (focused_client == event->seat_client) {
		wlr_cursor_set_surface(server->cursor, event->surface, event->hotspot_x, event->hotspot_y);
	}
    */
}

static void seat_pointer_focus_change(struct wl_listener *listener, void *data) {
    struct g_seat *seat = wl_container_of(listener, seat, pointer_focus_change);
	struct wlr_seat_pointer_focus_change_event *event = data;

    // Ignoring: graphio allows to use only one cursor

    /*
	if (event->new_surface == NULL) {
		wlr_cursor_set_xcursor(server->cursor, server->cursor_mgr, "default");
	}
    */
}

static void seat_request_set_selection(struct wl_listener *listener, void *data) {
    struct g_seat *seat = wl_container_of(listener, seat, request_set_selection);
	struct wlr_seat_request_set_selection_event *event = data;

	wlr_seat_set_selection(seat->wlr_seat, event->source, event->serial);
}

struct g_seat* g_seat_create(struct g_server *server) {
    struct g_seat *seat = malloc(sizeof(struct g_seat));
    seat->wlr_seat = wlr_seat_create(server->wl_display, DEFAULT_SEAT_NAME);

    seat->request_cursor.notify = seat_request_cursor;
	wl_signal_add(&seat->wlr_seat->events.request_set_cursor, &seat->request_cursor);

	seat->pointer_focus_change.notify = seat_pointer_focus_change;
	wl_signal_add(&seat->wlr_seat->pointer_state.events.focus_change, &seat->pointer_focus_change);

	seat->request_set_selection.notify = seat_request_set_selection;
	wl_signal_add(&seat->wlr_seat->events.request_set_selection, &seat->request_set_selection);

    return seat;
}

void g_seat_set_capabilities(struct g_seat *seat, uint32_t caps) {
    wlr_seat_set_capabilities(seat->wlr_seat, caps);
}

void g_seat_pointer_notify_enter(struct g_seat *seat, struct wlr_surface *surface, double sx, double sy) {
    wlr_seat_pointer_notify_enter(seat->wlr_seat, surface, sx, sy);
}

void g_seat_pointer_notify_motion(struct g_seat *seat, uint32_t time, double sx, double sy) {
    wlr_seat_pointer_notify_motion(seat->wlr_seat, time, sx, sy);
}

void g_seat_pointer_clear_focus(struct g_seat *seat) {
    wlr_seat_pointer_clear_focus(seat->wlr_seat);
}

void g_seat_pointer_notify_button(struct g_seat *seat, struct wlr_pointer_button_event *event) {
    wlr_seat_pointer_notify_button(seat->wlr_seat, event->time_msec, event->button, event->state);
}

void g_seat_pointer_notify_axis(struct g_seat *seat, struct wlr_pointer_axis_event *event) {
    wlr_seat_pointer_notify_axis(
        seat->wlr_seat,
        event->time_msec, 
        event->orientation, 
        event->delta,
        event->delta_discrete, 
        event->source, 
        event->relative_direction
    );
}

void g_seat_pointer_notify_frame(struct g_seat *seat) {
    wlr_seat_pointer_notify_frame(seat->wlr_seat);
}

void g_seat_enter_toplevel(struct g_seat *seat, struct g_toplevel *toplevel) {
    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat->wlr_seat);
    struct wlr_surface *current_toplevel_surface = toplevel->xdg_toplevel->base->surface;

	if (keyboard != NULL) {
		wlr_seat_keyboard_notify_enter(
			seat->wlr_seat, 
			current_toplevel_surface,
			keyboard->keycodes, 
			keyboard->num_keycodes, 
			&keyboard->modifiers
		);
	}
}

void g_seat_enter_surface(struct g_seat *seat, struct wlr_surface *surface) {
    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat->wlr_seat);

    wlr_seat_keyboard_notify_enter(
        seat->wlr_seat, 
        surface,
        keyboard->keycodes, 
        keyboard->num_keycodes, 
        &keyboard->modifiers
	);
}

void g_seat_set_keyboard(struct g_seat *seat, struct g_keyboard *keyboard) {
    wlr_seat_set_keyboard(seat->wlr_seat, keyboard->wlr_keyboard);
}

void g_seat_keyboard_notify_modifiers(struct g_seat *seat, struct g_keyboard *keyboard) {
    wlr_seat_keyboard_notify_modifiers(seat->wlr_seat, &keyboard->wlr_keyboard->modifiers);
}

void g_seat_keyboard_notify_key(struct g_seat *seat, struct wlr_keyboard_key_event *event) {
    wlr_seat_keyboard_notify_key(seat->wlr_seat, event->time_msec, event->keycode, event->state);
}

void g_seat_destroy(struct g_seat *seat) {
    wl_list_remove(&seat->request_cursor.link);
	wl_list_remove(&seat->pointer_focus_change.link);
	wl_list_remove(&seat->request_set_selection.link);
}