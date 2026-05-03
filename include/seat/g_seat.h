#ifndef G_SEAT_H
#define G_SEAT_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_seat.h>

struct g_server;
struct g_toplevel;
struct g_keyboard;

struct g_seat {
    struct wlr_seat *wlr_seat;

    struct wl_listener request_cursor;
	struct wl_listener pointer_focus_change;
	struct wl_listener request_set_selection;
};

struct g_seat* g_seat_create(struct g_server *server);

void g_seat_destroy(struct g_seat *seat);

void g_seat_set_capabilities(struct g_seat *seat, uint32_t caps);

void g_seat_pointer_notify_enter(struct g_seat *seat, struct wlr_surface *surface, double sx, double sy);
void g_seat_pointer_notify_motion(struct g_seat *seat, uint32_t time, double sx, double sy);
void g_seat_pointer_clear_focus(struct g_seat *seat);

void g_seat_pointer_notify_button(struct g_seat *seat, struct wlr_pointer_button_event *event);
void g_seat_pointer_notify_axis(struct g_seat *seat, struct wlr_pointer_axis_event *event);
void g_seat_pointer_notify_frame(struct g_seat *seat);

void g_seat_enter_toplevel(struct g_seat *seat, struct g_toplevel *toplevel);
void g_seat_enter_surface(struct g_seat *seat, struct wlr_surface *surface);

void g_seat_set_keyboard(struct g_seat *seat, struct g_keyboard *keyboard);
void g_seat_keyboard_notify_modifiers(struct g_seat *seat, struct g_keyboard *keyboard);
void g_seat_keyboard_notify_key(struct g_seat *seat, struct wlr_keyboard_key_event *event);

#endif