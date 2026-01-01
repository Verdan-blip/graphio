#define _POSIX_C_SOURCE 200112L

#include <wayland-util.h>
#include "include/g-server.h"
#include <stdlib.h>
#include <stdint.h>
#include <pixman.h>
#include <pixman-1/pixman.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_output.h>
#include <wlr/render/pass.h>
#include <wlr/util/log.h>
#include "../include/g-output.h"
#include "../include/g-dock-app.h"
#include "../include/g-dock-panel.h"
#include "../include/g-toplevel.h"
#include "../include/g-popup.h"

struct g_output* g_output_create(struct wlr_output *wlr_output, struct g_server *server) {
	wlr_output_init_render(wlr_output, server->allocator, server->renderer);

	struct wlr_output_state state;
	wlr_output_state_init(&state);
	wlr_output_state_set_enabled(&state, true);

	struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
	if (mode) {
		wlr_output_state_set_mode(&state, mode);
	}

	wlr_output_commit_state(wlr_output, &state);
	wlr_output_state_finish(&state);

    // Add to output layout
    wlr_output_layout_add_auto(server->output_layout, wlr_output);

    // Allocate memory
    struct g_output *output = calloc(1, sizeof(struct g_output));
	output->wlr_output = wlr_output;

    // Listeners
	output->frame_listener.notify = g_output_on_new_frame;
	wl_signal_add(&wlr_output->events.frame, &output->frame_listener);

    output->request_state_listener.notify = g_output_on_request_state;
    wl_signal_add(&wlr_output->events.request_state, &output->request_state_listener);

	output->destroy_listener.notify = g_output_on_destroy;
	wl_signal_add(&wlr_output->events.destroy, &output->destroy_listener);

    return output;
}

void g_output_on_new_frame(struct wl_listener *listener, void *data) {
    struct g_output *output = wl_container_of(listener, output, frame_listener);
    struct g_server *server = output->server;

    if (!output->wlr_output->enabled) return;

    struct wlr_output_state state;
    wlr_output_state_init(&state);

    struct wlr_render_pass *pass = wlr_output_begin_render_pass(output->wlr_output, &state, NULL);

    if (!pass) {
        wlr_output_state_finish(&state); 
        return;
    }

    // Wallpaper
    wlr_render_pass_add_rect(pass, &(struct wlr_render_rect_options) {
        .color = { 0.3f, 0.2f, 0.1f, 1.0f },
        .box = { 0, 0, output->wlr_output->width, output->wlr_output->height }
    });

    // Toplevels and it's popups
    struct g_toplevel *toplevel;
    wl_list_for_each_reverse(toplevel, &server->toplevels, link) {
        g_toplevel_on_render_pass(toplevel, pass);
    }

    // Dock panel
    g_dock_panel_on_render_pass(output->panel, pass);

    // Cursor
    g_cursor_on_render_pass(server->cursor, pass);

    wlr_render_pass_submit(pass);

    //pixman_region32_t damage;
    //pixman_region32_init_rect(&damage, 0, 0, output->wlr_output->width, output->wlr_output->height);
    //wlr_output_state_set_damage(&state, &damage);
    //pixman_region32_fini(&damage);

    if (!wlr_output_commit_state(output->wlr_output, &state)) {
        wlr_log(WLR_ERROR, "commit failed");
    }

    wlr_output_state_finish(&state);

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    // Toplevels send frame done
    wl_list_for_each(toplevel, &server->toplevels, link) {
        g_toplevel_send_frame_done(toplevel, &now);
    }

    // Popups send frame done
    struct g_popup *popup;
    wl_list_for_each(popup, &server->popups, link) {
        g_popup_send_frame_done(popup, &now);
    }
}

void g_output_on_request_state(struct wl_listener *listener, void *data) {
    struct g_output *output = wl_container_of(listener, output, request_state_listener);
	const struct wlr_output_event_request_state *event = data;
    
	wlr_output_commit_state(output->wlr_output, event->state);
}

void g_output_on_destroy(struct wl_listener *listener, void *data) {
    struct g_output *output = wl_container_of(listener, output, destroy_listener);

    g_dock_panel_destroy(output->panel);

    wlr_output_layout_remove(output->server->output_layout, output->wlr_output);

	wl_list_remove(&output->frame_listener.link);
    wl_list_remove(&output->request_state_listener.link);
	wl_list_remove(&output->destroy_listener.link);
	wl_list_remove(&output->link);
	free(output);
}