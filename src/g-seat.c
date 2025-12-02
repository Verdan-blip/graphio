#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_seat.h>
#include "../include/g-server.h"
#include "../include/g-seat.h"

struct g_seat* g_seat_create(struct g_server *server) {
    struct g_seat *seat = calloc(1, sizeof(struct g_seat));
    seat->server = server;
    
    seat->wlr_seat = wlr_seat_create(server->display, "seat0");

    seat->request_set_cursor_listener.notify = g_seat_set_request_set_cursor;
    wl_signal_add(&seat->wlr_seat->events.request_set_cursor, &seat->request_set_cursor_listener);

    return seat;
}

void g_seat_destroy(struct g_seat *seat) {
    wlr_seat_destroy(seat->wlr_seat);
    seat->wlr_seat = NULL;
    free(seat);
}

void g_seat_set_request_set_cursor(struct wl_listener *listener, void *data) {
    struct g_seat *seat = wl_container_of(listener, seat, request_set_cursor_listener);
	struct wlr_seat_pointer_request_set_cursor_event *event = data;

}