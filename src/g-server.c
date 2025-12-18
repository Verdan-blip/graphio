#define _POSIX_C_SOURCE 200112L
#include <wayland-util.h>
#include <stdlib.h>
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
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/render/allocator.h>
#include "../include/g-dock-app.h"
#include "../include/g-dock-panel.h"
#include "../include/g-toplevel-interaction.h"
#include "../include/g-toplevel.h"
#include "../include/g-popup.h"
#include "../include/g-server.h"
#include "../include/g-keyboard.h"
#include "../include/g-cursor.h"
#include "../include/g-output.h"
#include "../include/g-seat.h"
#include <wlr/render/gles2.h>

struct g_server* g_server_create() {
    struct g_server *server = calloc(1, sizeof(struct g_server));

    server->display = wl_display_create();
    if (!server->display) {
        wlr_log(WLR_ERROR, "Failed to create wlr_display");
        return NULL;
    }

    struct wl_event_loop *loop = wl_display_get_event_loop(server->display);
    if (!loop) {
        wlr_log(WLR_ERROR, "Failed to get event loop from wlr_display");
        return NULL;
    }

    server->backend = wlr_backend_autocreate(loop, NULL);
    if (!server->backend) {
        wlr_log(WLR_ERROR, "Failed to create wlr_backend");
        return NULL;
    }

    server->renderer = wlr_renderer_autocreate(server->backend);
    if (!server->renderer) {
        wlr_log(WLR_ERROR, "Failed to create wlr_renderer");
        return NULL;
    }

    wlr_renderer_init_wl_display(server->renderer, server->display);

    server->allocator = wlr_allocator_autocreate(server->backend, server->renderer);
	if (!server->allocator) {
		wlr_log(WLR_ERROR, "Failed to create wlr_allocator");
		return NULL;
	}

    struct wlr_compositor *compositor = wlr_compositor_create(server->display, 5, server->renderer);
    if (!compositor) {
        wlr_log(WLR_ERROR, "Failed to create wlr_compositor");
		return NULL;
    }

    struct wlr_subcompositor *subcompositor = wlr_subcompositor_create(server->display);
    if (!subcompositor) {
        wlr_log(WLR_ERROR, "Failed to create wlr_subcompositor");
		return NULL;
    }

    server->output_layout = wlr_output_layout_create(server->display);
    
    server->xdg_shell = wlr_xdg_shell_create(server->display, 3);

    // Prepare lists
    wl_list_init(&server->outputs);
    wl_list_init(&server->toplevels);
    wl_list_init(&server->popups);
    wl_list_init(&server->keyboards);

    // Seat
    struct g_seat *seat = g_seat_create(server);
    server->seat = seat;

    // Toplevel interaction
    struct g_toplevel_interaction *interaction = g_toplevel_interaction_create();
    server->toplevel_interaction = interaction;

    // Graphio cursor
    struct g_cursor *cursor = g_cursor_create(server);
    server->cursor = cursor;

    // Listeners
    server->new_input_listener.notify = g_server_on_new_input;
    wl_signal_add(&server->backend->events.new_input, &server->new_input_listener);

    server->new_output_listener.notify = g_server_on_new_output;
    wl_signal_add(&server->backend->events.new_output, &server->new_output_listener);

    // XDG Protocol
    server->new_toplevel_listener.notify = g_server_on_new_toplevel;
    wl_signal_add(&server->xdg_shell->events.new_toplevel, &server->new_toplevel_listener);

    server->new_popup_listener.notify = g_server_on_new_popup;
    wl_signal_add(&server->xdg_shell->events.new_popup, &server->new_popup_listener);

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

    g_cursor_destroy(server->cursor);
    g_seat_destroy(server->seat);
    g_toplevel_interaction_destroy(server->toplevel_interaction);

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
    struct g_dock_app *weston_terminal_app = g_dock_app_create(output, "weston-terminal", "Terminal", "/usr/bin/weston-terminal");

    g_dock_panel_add_app(panel, firefox_app);
    g_dock_panel_add_app(panel, gimp_app);
    g_dock_panel_add_app(panel, weston_terminal_app);
}

void g_server_on_new_input(struct wl_listener *listener, void *data) {
    struct g_server *server = wl_container_of(listener, server, new_input_listener);
    struct wlr_input_device *device = data;

    switch (device->type) {
        case WLR_INPUT_DEVICE_POINTER: {
            wlr_cursor_attach_input_device(server->cursor->wlr_cursor, device);
            break;
        }
        case WLR_INPUT_DEVICE_KEYBOARD: {
            struct g_keyboard *keyboard = g_keyboard_create(server, device);
            wl_list_insert(&server->keyboards, &keyboard->link);
            break;
        }
        default:
            break;
    }

    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
	if (!wl_list_empty(&server->keyboards)) {
		caps |= WL_SEAT_CAPABILITY_KEYBOARD;
	}
	wlr_seat_set_capabilities(server->seat->wlr_seat, caps);
}

void g_server_on_new_toplevel(struct wl_listener *listener, void *data) {
    struct g_server *server = wl_container_of(listener, server, new_toplevel_listener);
    struct wlr_xdg_toplevel *xdg_toplevel = data;

    struct g_toplevel *toplevel = g_toplevel_create(server, xdg_toplevel);
    wl_list_insert(&server->toplevels, &toplevel->link);
}

void g_server_on_new_popup(struct wl_listener *listener, void *data) {
    struct g_server *server = wl_container_of(listener, server, new_popup_listener);
    struct wlr_xdg_popup *xdg_popup = data;

    struct g_popup *popup = g_popup_create(server, xdg_popup);
    wl_list_insert(&server->popups, &popup->link);
}
