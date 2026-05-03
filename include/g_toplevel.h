#ifndef G_TOPLEVEL_H
#define G_TOPLEVEL_H

#include <stdint.h>
#include <wlr/util/box.h>
#include <wlr/types/wlr_foreign_toplevel_management_v1.h>
#include <wlr/types/wlr_xdg_shell.h>

#include "include/g_toplevel_handle.h"

struct g_server;

struct g_toplevel {
	struct wl_list link;

	char *app_id;

	bool maximized;
	bool focused;

	struct wlr_box restore_box;

	struct g_server *server;

	struct wlr_xdg_toplevel *xdg_toplevel;
	struct wlr_scene_tree *scene_tree;

	struct wl_listener map;
	struct wl_listener unmap;
	struct wl_listener commit;
	struct wl_listener destroy;
	struct wl_listener request_move;
	struct wl_listener request_resize;
	struct wl_listener request_maximize;
	struct wl_listener request_fullscreen;
	struct wl_listener set_app_id;
	struct wl_listener set_title;

	struct g_toplevel_handle *handle;
};

void g_toplevel_init(struct g_server *server, struct wlr_xdg_toplevel *toplevel);

void g_toplevel_focus(struct g_toplevel *toplevel);

struct g_toplevel* g_toplevel_at(
    struct g_server *server, 
    double lx, double ly,
	struct wlr_surface **surface, 
    double *sx, double *sy
);

#endif