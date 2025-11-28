#include <stdlib.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_output_layout.h>
#include "../include/g-cursor.h"

struct g_cursor* g_cursor_create(struct wlr_output_layout *output_layout) {
    struct g_cursor *cursor = calloc(1, sizeof(struct g_cursor));

    cursor->wlr_cursor = wlr_cursor_create();
    wlr_cursor_attach_output_layout(cursor->wlr_cursor, output_layout);

    cursor->cursor_mgr = wlr_xcursor_manager_create(NULL, 24);
    cursor->cursor_mode = TINYWL_CURSOR_PASSTHROUGH;

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

}

void g_cursor_on_absolute_motion(struct wl_listener *listener, void *data) {

}

void g_cursor_on_button(struct wl_listener *listener, void *data) {

}

void g_cursor_on_axis(struct wl_listener *listener, void *data) {

}

void g_cursor_on_new_frame(struct wl_listener *listener, void *data) {

}