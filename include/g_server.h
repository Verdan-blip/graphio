#ifndef G_SERVER_H
#define G_SERVER_H

#include <wlr/types/wlr_foreign_toplevel_management_v1.h>

#include "include/g_layer_surface.h"
#include "include/toplevel/g_toplevel.h"

struct g_layer_surface;
struct g_seat;
struct g_cursor;

struct g_server {
	struct wl_display *wl_display;
	struct wlr_backend *backend;
	struct wlr_renderer *renderer;
	struct wlr_allocator *allocator;

	struct wlr_scene *scene;
	struct wlr_scene_output_layout *scene_layout;

	struct wlr_scene_tree *background_tree;
	struct wlr_scene_tree *main_tree;
	struct wlr_scene_tree *foregound_tree;

	struct wlr_surface *switcher_surface;

	struct wlr_xdg_shell *xdg_shell;
	struct wl_listener new_xdg_toplevel;
	struct wl_listener new_xdg_popup;

	struct wl_list toplevels;
	struct wl_list keyboards;
	struct g_seat *seat;
	struct g_cursor *cursor;

	struct g_toplevel *current_toplevel;

	struct wlr_layer_shell_v1 *layer_shell;
	struct wl_list layer_surfaces;
	struct wl_listener new_layer;

	struct wl_listener new_input;

	struct g_toplevel *grabbed_toplevel;
	double grab_x, grab_y;
	struct wlr_box grab_geobox;
	uint32_t resize_edges;

	struct wlr_output_layout *output_layout;
	struct wl_list outputs;
	struct wl_listener new_output;

	struct wlr_foreign_toplevel_manager_v1 *toplevel_manager;
};

struct wlr_output* g_server_get_current_output(struct g_server *server);

#endif