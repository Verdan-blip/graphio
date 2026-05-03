#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/util/box.h>
#include <wayland-util.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wayland-server-protocol.h>
#include <wlr/util/log.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <wlr/types/wlr_foreign_toplevel_management_v1.h>

#include "include/g_launcher.h"
#include "include/g_layer_surface.h"
#include "include/g_server.h"
#include "include/toplevel/g_toplevel.h"
#include "include/output/g_output.h"
#include "include/popup/g_popup.h"
#include "include/keyboard/g_keyboard.h"
#include "include/cursor/g_cursor.h"
#include "include/seat/g_seat.h"

struct wlr_output* g_server_get_current_output(struct g_server *server) {
	struct g_cursor *cursor = server->cursor;

	return wlr_output_layout_output_at(
		server->output_layout, 
		cursor->wlr_cursor->x,
		cursor->wlr_cursor->y
	);
}

static void server_new_input(struct wl_listener *listener, void *data) {
	struct g_server *server = wl_container_of(listener, server, new_input);
	struct wlr_input_device *device = data;

	switch (device->type) {
	case WLR_INPUT_DEVICE_KEYBOARD: {
		struct g_keyboard *keyboard = g_keyboard_create(server, device);
		wl_list_insert(&server->keyboards, &keyboard->link);
		break;
	}
	case WLR_INPUT_DEVICE_POINTER: {
		g_cursor_attach_input_device(server->cursor, device);
		break;
	}
	default:
		break;
	}

	uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
	if (!wl_list_empty(&server->keyboards)) {
		caps |= WL_SEAT_CAPABILITY_KEYBOARD;
	}

	g_seat_set_capabilities(server->seat, caps);
}

static void server_new_output(struct wl_listener *listener, void *data) {
	struct g_server *server = wl_container_of(listener, server, new_output);
	struct wlr_output *wlr_output = data;

	struct g_output *output = g_output_create(server, wlr_output);
	wl_list_insert(&server->outputs, &output->link);
}

static void server_new_xdg_popup(struct wl_listener *listener, void *data) {
	struct g_server *server = wl_container_of(listener, server, new_xdg_popup);
	struct wlr_xdg_popup *xdg_popup = data;

	g_popup_init(server, xdg_popup);
}

static void server_new_xdg_toplevel(struct wl_listener *listener, void *data) {
	struct g_server *server = wl_container_of(listener, server, new_xdg_toplevel);
	struct wlr_xdg_toplevel *xdg_toplevel = data;

	g_toplevel_init(server, xdg_toplevel);
}

static void server_new_wlr_layer_surface(struct wl_listener *listener, void *data) {
	struct g_server *server = wl_container_of(listener, server, new_layer);
    struct wlr_layer_surface_v1 *wlr_layer_surface = data;

	g_layer_surface_init(server, wlr_layer_surface);
}

int main(int argc, char *argv[]) {
	wlr_log_init(WLR_DEBUG, NULL);

	struct g_server server = {0};
	server.wl_display = wl_display_create();

	server.backend = wlr_backend_autocreate(wl_display_get_event_loop(server.wl_display), NULL);
	if (server.backend == NULL) {
		wlr_log(WLR_ERROR, "failed to create wlr_backend");
		return 1;
	}

	server.renderer = wlr_renderer_autocreate(server.backend);
	if (server.renderer == NULL) {
		wlr_log(WLR_ERROR, "failed to create wlr_renderer");
		return 1;
	}

	wlr_renderer_init_wl_display(server.renderer, server.wl_display);

	server.allocator = wlr_allocator_autocreate(server.backend, server.renderer);

	if (server.allocator == NULL) {
		wlr_log(WLR_ERROR, "failed to create wlr_allocator");
		return 1;
	}

	wlr_compositor_create(server.wl_display, 5, server.renderer);
	wlr_subcompositor_create(server.wl_display);
	wlr_data_device_manager_create(server.wl_display);

	server.output_layout = wlr_output_layout_create(server.wl_display);

	wl_list_init(&server.outputs);
	server.new_output.notify = server_new_output;
	wl_signal_add(&server.backend->events.new_output, &server.new_output);

	server.scene = wlr_scene_create();
	server.scene_layout = wlr_scene_attach_output_layout(server.scene, server.output_layout);

	server.background_tree = wlr_scene_tree_create(&server.scene->tree);
	server.foregound_tree = wlr_scene_tree_create(&server.scene->tree);
	server.main_tree = wlr_scene_tree_create(&server.scene->tree);

	wlr_scene_node_set_enabled(&server.foregound_tree->node, false);

	wlr_scene_node_raise_to_top(&server.foregound_tree->node);
	wlr_scene_node_lower_to_bottom(&server.background_tree->node);

	wlr_scene_node_place_below(&server.main_tree->node, &server.foregound_tree->node);

	// Toplevel
	server.current_toplevel = NULL;
	
	wl_list_init(&server.toplevels);
	server.xdg_shell = wlr_xdg_shell_create(server.wl_display, 3);
	server.new_xdg_toplevel.notify = server_new_xdg_toplevel;
	wl_signal_add(&server.xdg_shell->events.new_toplevel, &server.new_xdg_toplevel);
	server.new_xdg_popup.notify = server_new_xdg_popup;
	wl_signal_add(&server.xdg_shell->events.new_popup, &server.new_xdg_popup);

	// Layer shell
	server.layer_shell = wlr_layer_shell_v1_create(server.wl_display, 3);
	
	wl_list_init(&server.layer_surfaces);
	server.new_layer.notify = server_new_wlr_layer_surface;
	wl_signal_add(&server.layer_shell->events.new_surface, &server.new_layer);

	// Cursor
	server.cursor = g_cursor_create(&server);

	// Keyboards
	wl_list_init(&server.keyboards);
	server.new_input.notify = server_new_input;
	wl_signal_add(&server.backend->events.new_input, &server.new_input);

	// Seat
	server.seat = g_seat_create(&server);

	// Foreign toplevel manager
	server.toplevel_manager = wlr_foreign_toplevel_manager_v1_create(server.wl_display);

	/* Add a Unix socket to the Wayland display. */
	const char *socket = wl_display_add_socket_auto(server.wl_display);
	if (!socket) {
		wlr_backend_destroy(server.backend);
		return 1;
	}

	// Start the backend
	if (!wlr_backend_start(server.backend)) {
		wlr_backend_destroy(server.backend);
		wl_display_destroy(server.wl_display);
		return 1;
	}

	setenv("WAYLAND_DISPLAY", socket, true);

	// Launch weston-terminal as start application
	g_launch("weston-terminal");

	wlr_log(WLR_INFO, "Running Wayland compositor on WAYLAND_DISPLAY=%s", socket);
	wl_display_run(server.wl_display);

	wl_display_destroy_clients(server.wl_display);

	wl_list_remove(&server.new_layer.link);

	wl_list_remove(&server.new_xdg_toplevel.link);
	wl_list_remove(&server.new_xdg_popup.link);

	g_cursor_destroy(server.cursor);
	g_seat_destroy(server.seat);

	wl_list_remove(&server.new_input.link);
	wl_list_remove(&server.new_output.link);

	wlr_scene_node_destroy(&server.scene->tree.node);
	wlr_allocator_destroy(server.allocator);
	wlr_renderer_destroy(server.renderer);
	wlr_backend_destroy(server.backend);
	wl_display_destroy(server.wl_display);
	return 0;
}
