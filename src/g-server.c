#define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <wlr/util/log.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/render/allocator.h>
#include "../include/g-dock-app.h"
#include "../include/g-dock-panel.h"
#include "../include/g-server.h"
#include "../include/g-cursor.h"
#include "../include/g-output.h"
#include "../include/g-seat.h"

struct g_server* g_server_create() {
    struct g_server *server = calloc(1, sizeof(struct g_server));

    server->display = wl_display_create();
    if (!server->display) {
        wlr_log(WLR_ERROR, "Failed to create display");
        return NULL;
    }

    struct wl_event_loop *loop = wl_display_get_event_loop(server->display);
    if (!loop) {
        wlr_log(WLR_ERROR, "Failed to get event loop from display");
        return NULL;
    }

    server->backend = wlr_backend_autocreate(loop, NULL);
    if (!server->backend) {
        wlr_log(WLR_ERROR, "Failed to create backend");
        return NULL;
    }

    server->renderer = wlr_renderer_autocreate(server->backend);
    if (!server->renderer) {
        wlr_log(WLR_ERROR, "Failed to create renderer");
        return NULL;
    }

    wlr_renderer_init_wl_display(server->renderer, server->display);

    server->allocator = wlr_allocator_autocreate(server->backend, server->renderer);
	if (!server->allocator) {
		wlr_log(WLR_ERROR, "Failed to create allocator");
		return NULL;
	}

    server->output_layout = wlr_output_layout_create(server->display);

    // Prepare lists
    wl_list_init(&server->outputs);

    // Graphio Cursor
    struct g_cursor *cursor = g_cursor_create(server);
    server->cursor = cursor;

    // Seat
    struct g_seat *seat = g_seat_create(server);
    server->seat = seat;

    // Listeners
    server->new_input_listener.notify = g_server_on_new_input;
    wl_signal_add(&server->backend->events.new_input, &server->new_input_listener);

    server->new_output_listener.notify = g_server_on_new_output;
    wl_signal_add(&server->backend->events.new_output, &server->new_output_listener);

    return server;
}

void g_server_run(struct g_server *server) {
    const char *socket = wl_display_add_socket_auto(server->display);
	if (!socket) {
        wlr_log(WLR_ERROR, "Failed to open socket");
		wlr_backend_destroy(server->backend);
		return;
	}

	if (!wlr_backend_start(server->backend)) {
        wlr_log(WLR_ERROR, "Failed to start backend");
		wlr_backend_destroy(server->backend);
		wl_display_destroy(server->display);
		return;
	}

	setenv("WAYLAND_DISPLAY", socket, true);
	wlr_log(WLR_INFO, "Running Wayland compositor on WAYLAND_DISPLAY=%s", socket);

	wl_display_run(server->display);
}

void g_server_destroy(struct g_server *server) {
    wl_display_destroy_clients(server->display);

    // Graphio structures
    g_cursor_destroy(server->cursor);
    g_seat_destroy(server->seat);

    wl_list_remove(&server->new_output_listener.link);
    wl_list_remove(&server->new_input_listener.link);

    wlr_allocator_destroy(server->allocator);
	wlr_renderer_destroy(server->renderer);
	wlr_backend_destroy(server->backend);
	wl_display_destroy(server->display);

    free(server);
}

void g_server_on_new_output(struct wl_listener *listener, void *data) {
    struct g_server *server = wl_container_of(listener, server, new_output_listener);
    struct wlr_output *wlr_output = data;

    struct g_output *output = g_output_create(wlr_output, server);
    output->server = server;

    wl_list_insert(&server->outputs, &output->link);

    // Dock panel
    struct g_dock_panel *panel = g_dock_panel_create(output);
    output->panel = panel;

    struct g_dock_app *firefox_app = g_dock_app_create(output, "firefox", "Firefox", "/usr/bin/firefox");
    struct g_dock_app *gimp_app = g_dock_app_create(output, "gimp", "Gimp", "/usr/bin/gimp");

    g_dock_panel_add_app(panel, firefox_app);
    g_dock_panel_add_app(panel, gimp_app);
}

void g_server_on_new_input(struct wl_listener *listener, void *data) {
    struct g_server *server = wl_container_of(listener, server, new_input_listener);
    struct wlr_input_device *device = data;

    switch (device->type) {
        case WLR_INPUT_DEVICE_POINTER:
            wlr_cursor_attach_input_device(server->cursor->wlr_cursor, device);
            break;
        default:
            break;
    }

    wlr_seat_set_capabilities(server->seat->wlr_seat, WL_SEAT_CAPABILITY_POINTER);
}
