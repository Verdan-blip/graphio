#ifndef G_SEAT_H
#define G_SEAT_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_seat.h>

struct g_server;

struct g_seat {
    struct g_server *server;

    struct wlr_seat *wlr_seat;

    struct wl_listener request_set_cursor_listener;
};

struct g_seat* g_seat_create(struct g_server *server);
void g_seat_destroy(struct g_seat *seat);

void g_seat_set_request_set_cursor(struct wl_listener *listener, void *data);

#endif
