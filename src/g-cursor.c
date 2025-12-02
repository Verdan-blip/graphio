#include <stdlib.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/interfaces/wlr_pointer.h>
#include <wlr/util/log.h>
#include <drm/drm_fourcc.h>
#include <stdio.h>
#include "../include/g-server.h"
#include "../include/g-cursor.h"

struct g_cursor* g_cursor_create(struct g_server *server) {
    struct g_cursor *cursor = calloc(1, sizeof(struct g_cursor));
    cursor->server = server;

    cursor->wlr_cursor = wlr_cursor_create();
    if (!cursor->wlr_cursor) {
        wlr_log(WLR_ERROR, "Failed to create wlr_cursor");
    }

    wlr_cursor_attach_output_layout(cursor->wlr_cursor, server->output_layout);

    cursor->cursor_mgr = wlr_xcursor_manager_create(NULL, 24);
    cursor->cursor_mode = TINYWL_CURSOR_PASSTHROUGH;

    wlr_xcursor_manager_load(cursor->cursor_mgr, 1);

    struct wlr_xcursor *xcursor = wlr_xcursor_manager_get_xcursor(cursor->cursor_mgr, "left_ptr", 1);
    struct wlr_xcursor_image *img = xcursor->images[0];

    cursor->texture = wlr_texture_from_pixels(
        server->renderer,
        DRM_FORMAT_ARGB8888,
        img->width * 4,
        img->width,
        img->height,
        img->buffer
    );

    if (!cursor->texture) {
        wlr_log(WLR_ERROR, "Failed to load xcursor texture");
    }

    cursor->cursor_motion_listener.notify = g_cursor_on_motion;
    wl_signal_add(&cursor->wlr_cursor->events.motion, &cursor->cursor_motion_listener);

    cursor->cursor_motion_absolute_listener.notify = g_cursor_on_absolute_motion;
    wl_signal_add(&cursor->wlr_cursor->events.motion_absolute, &cursor->cursor_motion_absolute_listener);

    cursor->cursor_button_listener.notify = g_cursor_on_button;
    wl_signal_add(&cursor->wlr_cursor->events.button, &cursor->cursor_button_listener);

    cursor->cursor_axis_listener.notify = g_cursor_on_axis;
    wl_signal_add(&cursor->wlr_cursor->events.axis, &cursor->cursor_axis_listener);

    cursor->cursor_frame_listener.notify = g_cursor_on_new_frame;
    wl_signal_add(&cursor->wlr_cursor->events.frame, &cursor->cursor_frame_listener);

    return cursor;
}

void g_cursor_destroy(struct g_cursor *cursor) {
    wl_list_remove(&cursor->cursor_motion_listener.link);
	wl_list_remove(&cursor->cursor_motion_absolute_listener.link);
	wl_list_remove(&cursor->cursor_button_listener.link);
	wl_list_remove(&cursor->cursor_axis_listener.link);
	wl_list_remove(&cursor->cursor_frame_listener.link);

    wlr_xcursor_manager_destroy(cursor->cursor_mgr);
    wlr_cursor_destroy(cursor->wlr_cursor);

    cursor->server = NULL;

    free(cursor);
}

void g_cursor_on_motion(struct wl_listener *listener, void *data) {
    struct g_cursor *cursor = wl_container_of(listener, cursor, cursor_motion_listener);
    struct wlr_pointer_motion_event *event = data;

    wlr_log(WLR_INFO, "%f, %f", event->delta_x, event->delta_y);

    wlr_cursor_move(cursor->wlr_cursor, &event->pointer->base, event->delta_x, event->delta_y);
}

void g_cursor_on_absolute_motion(struct wl_listener *listener, void *data) {
	struct g_cursor *cursor = wl_container_of(listener, cursor, cursor_motion_absolute_listener);
	struct wlr_pointer_motion_absolute_event *event = data;

    wlr_log(WLR_INFO, "%f, %f", event->x, event->y);
    
	wlr_cursor_warp_absolute(cursor->wlr_cursor, &event->pointer->base, event->x, event->y);
}

void g_cursor_on_button(struct wl_listener *listener, void *data) {

}

void g_cursor_on_axis(struct wl_listener *listener, void *data) {

}

void g_cursor_on_new_frame(struct wl_listener *listener, void *data) {

}

// Render contract
void g_cursor_on_render_pass(struct g_cursor *cursor, struct wlr_render_pass *pass) {
    const float cursor_alpha = 1.0f;

    wlr_render_pass_add_texture(pass, &(struct wlr_render_texture_options) {
        .alpha = &cursor_alpha,
        .texture = cursor->texture,
        .src_box = { 0, 0, 24, 24 },
        .dst_box = { cursor->wlr_cursor->x, cursor->wlr_cursor->y, 24, 24 }
    });
}