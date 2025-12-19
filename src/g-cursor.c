#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wayland-server-protocol.h>
#include <wayland-util.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/interfaces/wlr_pointer.h>
#include <wlr/util/log.h>
#include <drm/drm_fourcc.h>
#include "../include/g-server.h"
#include "../include/g-output.h"
#include "../include/g-dock-panel.h"
#include "../include/g-cursor.h"
#include "../include/g-toplevel.h"
#include "../include/g-toplevel-interaction.h"
#include "include/g-popup.h"

struct g_cursor* g_cursor_create(struct g_server *server) {
    struct g_cursor *cursor = calloc(1, sizeof(struct g_cursor));
    cursor->server = server;

    cursor->wlr_cursor = wlr_cursor_create();
    if (!cursor->wlr_cursor) {
        wlr_log(WLR_ERROR, "Failed to create wlr_cursor");
    }

    wlr_cursor_attach_output_layout(cursor->wlr_cursor, server->output_layout);

    cursor->cursor_mgr = wlr_xcursor_manager_create(NULL, 24);
    cursor->cursor_mode = CURSOR_PASSTHROUGH;

    wlr_xcursor_manager_load(cursor->cursor_mgr, 1);

    g_cursor_reset_mode(cursor);

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

static void g_cursor_find_and_move_toplevel_if_grabbed(struct g_cursor *cursor, uint32_t time) {
    struct g_server *server = cursor->server;
    struct g_toplevel_interaction *interaction = server->toplevel_interaction;

    if (interaction->grabbed_toplevel) {
        interaction->grabbed_toplevel->pos_x = cursor->wlr_cursor->x - interaction->grab_pos_x;
        interaction->grabbed_toplevel->pos_y = cursor->wlr_cursor->y - interaction->grab_pos_y;
    }
}

static void g_cursor_find_and_resize_toplevel_if_grabbed(struct g_cursor *cursor, uint32_t time) {
    struct g_server *server = cursor->server;
    struct g_toplevel_interaction *interaction = server->toplevel_interaction;

    if (interaction->grabbed_toplevel) {
        double border_x = server->cursor->wlr_cursor->x - interaction->grab_pos_x;
	    double border_y = server->cursor->wlr_cursor->y - interaction->grab_pos_y;

        int new_left = interaction->grabbed_client_box.x;
	    int new_right = interaction->grabbed_client_box.x + interaction->grabbed_client_box.width;
	    int new_top = interaction->grabbed_client_box.y;
	    int new_bottom = interaction->grabbed_client_box.y + interaction->grabbed_client_box.height;

        if (interaction->resize_edges & WLR_EDGE_TOP) {
            new_top = border_y;
            if (new_top >= new_bottom) {
                new_top = new_bottom - 1;
            }
        } else if (interaction->resize_edges & WLR_EDGE_BOTTOM) {
            new_bottom = border_y;
            if (new_bottom <= new_top) {
                new_bottom = new_top + 1;
            }
        }
        if (interaction->resize_edges & WLR_EDGE_LEFT) {
            new_left = border_x;
            if (new_left >= new_right) {
                new_left = new_right - 1;
            }
        } else if (interaction->resize_edges & WLR_EDGE_RIGHT) {
            new_right = border_x;
            if (new_right <= new_left) {
                new_right = new_left + 1;
            }
        }

        struct wlr_box *geo_box = &interaction->grabbed_toplevel->xdg_toplevel->base->geometry;

        interaction->grabbed_toplevel->pos_x = new_left - geo_box->x;
        interaction->grabbed_toplevel->pos_y = new_top - geo_box->y;

        int new_width = new_right - new_left;
        int new_height = new_bottom - new_top;

        wlr_xdg_toplevel_set_size(
            interaction->grabbed_toplevel->xdg_toplevel, 
            new_width, 
            new_height
        );
    }
}

static void g_cursor_find_and_enter_toplevel_if_focused(struct g_cursor *cursor, uint32_t time) {
    struct g_server *server = cursor->server;

    if (!wl_list_empty(&server->toplevels)) {
        double surface_x, surface_y;
        
        struct g_toplevel *toplevel = g_toplevel_at(
            &server->toplevels,
            cursor->wlr_cursor->x,
            cursor->wlr_cursor->y,
            &surface_x, &surface_y
        );

        if (toplevel) {
            if (toplevel->focused && toplevel->surface) {
                wlr_seat_pointer_notify_enter(
                    server->seat->wlr_seat, 
                    toplevel->surface, 
                    cursor->wlr_cursor->x, 
                    cursor->wlr_cursor->y
                );
                wlr_seat_pointer_notify_motion(
                    server->seat->wlr_seat,
                    time,
                    surface_x, surface_y
                );
            } else {
                wlr_seat_pointer_clear_focus(server->seat->wlr_seat);
            }
        }
    }
}

static void g_cursor_process_motion(struct g_cursor *cursor, uint32_t time) {
    switch (cursor->cursor_mode) {
        case CURSOR_RESIZE: 
            g_cursor_find_and_resize_toplevel_if_grabbed(cursor, time);
            break;
        case CURSOR_MOVE:
            g_cursor_find_and_move_toplevel_if_grabbed(cursor, time);
            break;
        default:
            break;
    }

    g_cursor_find_and_enter_toplevel_if_focused(cursor, time);
}

void g_cursor_on_motion(struct wl_listener *listener, void *data) {
    struct g_cursor *cursor = wl_container_of(listener, cursor, cursor_motion_listener);
    struct wlr_pointer_motion_event *event = data;

    wlr_cursor_move(
        cursor->wlr_cursor, 
        &event->pointer->base, 
        event->delta_x, event->delta_y
    );

    g_cursor_process_motion(cursor, event->time_msec);
}

void g_cursor_on_absolute_motion(struct wl_listener *listener, void *data) {
	struct g_cursor *cursor = wl_container_of(listener, cursor, cursor_motion_absolute_listener);
	struct wlr_pointer_motion_absolute_event *event = data;
    
	wlr_cursor_warp_absolute(
        cursor->wlr_cursor, 
        &event->pointer->base, 
        event->x, event->y
    );

    g_cursor_process_motion(cursor, event->time_msec);
}

void g_cursor_on_button(struct wl_listener *listener, void *data) {
    struct g_cursor *cursor = wl_container_of(listener, cursor, cursor_button_listener);
    struct g_server *server = cursor->server;

	struct wlr_pointer_button_event *event = data;

    wlr_seat_pointer_notify_button(
		server->seat->wlr_seat,
		event->time_msec, 
		event->button, 
		event->state
	);

    struct g_output *output;
    wl_list_for_each(output, &server->outputs, link) {
        bool consumed = g_dock_panel_consume_cursor_button_event(
            output->panel, 
            cursor->wlr_cursor->x, cursor->wlr_cursor->y, 
            event
        );

        if (consumed) return;
    }

    struct g_toplevel *toplevel;
    wl_list_for_each(toplevel, &server->toplevels, link) {
        bool consumed = g_toplevel_consume_cursor_button_event(
            toplevel, 
            cursor->wlr_cursor->x, cursor->wlr_cursor->y, 
            event
        );

        if (consumed) return;
    }

    switch (event->state) {
        case WL_POINTER_BUTTON_STATE_RELEASED:
            g_toplevel_interacton_reset(server->toplevel_interaction);
            g_cursor_reset_mode(cursor);
            break;
        case WL_POINTER_BUTTON_STATE_PRESSED: 
            break;
        default:
            break;
    }
}

void g_cursor_on_axis(struct wl_listener *listener, void *data) {

}

void g_cursor_on_new_frame(struct wl_listener *listener, void *data) {

}

static void g_cursor_set_xcursor_image_by_name(struct g_cursor *cursor, const char* name) {
    struct wlr_xcursor *xcursor = wlr_xcursor_manager_get_xcursor(cursor->cursor_mgr, name, 1);
    struct wlr_xcursor_image *img = xcursor->images[0];

    cursor->texture = wlr_texture_from_pixels(
        cursor->server->renderer,
        DRM_FORMAT_ARGB8888,
        img->width * 4,
        img->width,
        img->height,
        img->buffer
    );

    if (!cursor->texture) {
        wlr_log(WLR_ERROR, "Failed to load xcursor texture");
    }
}

void g_cursor_set_cursor_mode_resize_horizontal(struct g_cursor *cursor) {
    cursor->cursor_mode = CURSOR_RESIZE;
    g_cursor_set_xcursor_image_by_name(cursor, "left_side");
}

void g_cursor_set_cursor_mode_resize_vertical(struct g_cursor *cursor) {
    cursor->cursor_mode = CURSOR_RESIZE;
    g_cursor_set_xcursor_image_by_name(cursor, "top_side");
}

void g_cursor_set_cursor_mode_move(struct g_cursor *cursor) {
    cursor->cursor_mode = CURSOR_MOVE;
    g_cursor_set_xcursor_image_by_name(cursor, "left_ptr");
}

void g_cursor_reset_mode(struct g_cursor *cursor) {
    cursor->cursor_mode = CURSOR_PASSTHROUGH;
    g_cursor_set_xcursor_image_by_name(cursor, "left_ptr");
}

// Render contract
void g_cursor_on_render_pass(struct g_cursor *cursor, struct wlr_render_pass *pass) {
    if (!cursor) {
        wlr_log(WLR_INFO, "Graphio cursor has not been attached yet, skipping");
    }

    const float cursor_alpha = 1.0f;

    wlr_render_pass_add_texture(pass, &(struct wlr_render_texture_options) {
        .alpha = &cursor_alpha,
        .texture = cursor->texture,
        .src_box = { 0, 0, 24, 24 },
        .dst_box = { cursor->wlr_cursor->x, cursor->wlr_cursor->y, 24, 24 }
    });
}