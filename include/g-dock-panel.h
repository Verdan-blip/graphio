#ifndef G_DOCK_H
#define G_DOCK_H

#include <wayland-server-core.h>
#include <wlr/render/wlr_renderer.h>
#include "../include/g-server.h"
#include "../include/g-icon.h"
#include "../include/components/paddings.h"

struct g_dock_app;

struct g_dock_panel {
    struct g_output *output;

    struct wl_list apps;
    struct wlr_scene_tree *scene_tree;
    struct wlr_scene_rect *background;
    struct paddings paddings;
    int apps_padding_between;
    int pos_x, pos_y;
    int width, height;
};

struct g_dock_panel* g_dock_panel_create(struct g_output *output);
void g_dock_panel_update(struct g_dock_panel *panel);
void g_dock_panel_add_app(struct g_dock_panel *panel, struct g_dock_app *app);
void g_dock_panel_destroy(struct g_dock_panel *panel);

// Contract
void g_dock_panel_on_render_pass(struct g_dock_panel *panel, struct wlr_render_pass *pass);

bool g_dock_panel_consume_cursor_button_event(
    struct g_dock_panel *panel, 
    double x, 
    double y, 
    struct wlr_pointer_button_event *event
);

#endif