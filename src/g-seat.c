#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_seat.h>
#include "../include/g-server.h"
#include "../include/g-seat.h"

struct g_seat* g_seat_create(struct g_server *server) {
    struct g_seat *seat = calloc(1, sizeof(struct g_seat));
    seat->server = server;
    
    seat->wlr_seat = wlr_seat_create(server->display, "seat0");

    seat->request_set_cursor_listener.notify = g_seat_on_request_set_cursor;
    wl_signal_add(&seat->wlr_seat->events.request_set_cursor, &seat->request_set_cursor_listener);

    return seat;
}

void g_seat_destroy(struct g_seat *seat) {
    wlr_seat_destroy(seat->wlr_seat);
    seat->wlr_seat = NULL;
    free(seat);
}

void g_seat_on_request_set_cursor(struct wl_listener *listener, void *data) {
    struct g_seat *seat = wl_container_of(listener, seat, request_set_cursor_listener);
    struct g_server *server = seat->server;

	struct wlr_seat_pointer_request_set_cursor_event *event = data;
	struct wlr_seat_client *focused_client = seat->wlr_seat->pointer_state.focused_client;

	if (focused_client == event->seat_client) {
		wlr_cursor_set_surface(
            server->cursor->wlr_cursor, 
            event->surface,
				event->hotspot_x, 
                event->hotspot_y
        );
	}

}

void g_seat_on_pointer_focus_change(struct wl_listener *listener, void *data) {
    struct g_seat *seat = wl_container_of(listener, seat, pointer_focus_change_listener);
    struct g_server *server = seat->server;

	struct wlr_seat_pointer_focus_change_event *event = data;

	if (event->new_surface == NULL) {
		wlr_cursor_set_xcursor(
            server->cursor->wlr_cursor, 
            server->cursor->cursor_mgr, 
            "default"
        );
	}

}

void g_seat_on_request_set_selection(struct wl_listener *listener, void *data) {
    struct g_seat *seat = wl_container_of(listener, seat, pointer_focus_change_listener);
    struct g_server *server = seat->server;

    struct wlr_seat_request_set_selection_event *event = data;
    //wlr_seat_set_selection(server->seat, event->source, event->serial);
}