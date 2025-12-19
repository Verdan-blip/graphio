#ifndef G_CURSOR_H
#define G_CURSOR_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/render/wlr_renderer.h>

struct g_server;

enum g_cursor_mode {
	CURSOR_PASSTHROUGH,
	CURSOR_MOVE,
	CURSOR_RESIZE
};

struct g_cursor {
	struct g_server *server; 

    struct wlr_cursor *wlr_cursor;
	struct wlr_xcursor_manager *cursor_mgr;
	enum g_cursor_mode cursor_mode;

	struct wlr_texture *texture;

	struct wl_listener cursor_motion_listener;
	struct wl_listener cursor_motion_absolute_listener;
	struct wl_listener cursor_button_listener;
	struct wl_listener cursor_axis_listener;
	struct wl_listener cursor_frame_listener;
};

struct g_cursor* g_cursor_create(struct g_server *server);
void g_cursor_destroy(struct g_cursor *cursor);

void g_cursor_on_motion(struct wl_listener *listener, void *data);
void g_cursor_on_absolute_motion(struct wl_listener *listener, void *data);
void g_cursor_on_motion(struct wl_listener *listener, void *data);
void g_cursor_on_button(struct wl_listener *listener, void *data);
void g_cursor_on_axis(struct wl_listener *listener, void *data);
void g_cursor_on_new_frame(struct wl_listener *listener, void *data);

void g_cursor_set_cursor_mode_resize_horizontal(struct g_cursor *cursor);
void g_cursor_set_cursor_mode_resize_vertical(struct g_cursor *cursor);
void g_cursor_set_cursor_mode_move(struct g_cursor *cursor);
void g_cursor_reset_mode(struct g_cursor *cursor);

// Render contract
void g_cursor_on_render_pass(struct g_cursor *cursor, struct wlr_render_pass *pass);

#endif