#ifndef G_CURSOR_H
#define G_CURSOR_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_input_device.h>

struct g_server;

enum g_cursor_mode {
	G_CURSOR_PASSTHROUGH,
	G_CURSOR_MOVE,
	G_CURSOR_RESIZE,
};

struct g_cursor {
    struct g_server *server;

    struct wlr_cursor *wlr_cursor;
	struct wlr_xcursor_manager *cursor_mgr;
    enum g_cursor_mode cursor_mode;

	struct wl_listener cursor_motion;
	struct wl_listener cursor_motion_absolute;
	struct wl_listener cursor_button;
	struct wl_listener cursor_axis;
	struct wl_listener cursor_frame;
};

struct g_cursor* g_cursor_create(struct g_server *server);
void g_cursor_destroy(struct g_cursor *cursor);

void g_cursor_reset_mode(struct g_cursor *cursor);

void g_cursor_attach_input_device(struct g_cursor *cursor, struct wlr_input_device *device);

#endif
