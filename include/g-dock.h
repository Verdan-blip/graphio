#ifndef G_DOCK_H
#define G_DOCK_H

#include <wayland-server-core.h>
#include <wlr/render/wlr_renderer.h>
#include "../include/g-server.h"
#include "../include/g-icon.h"
#include "../include/components/paddings.h"

struct g_dock_app {
    struct wl_list link;
    char *id;
    char *title;
    int pos_x, pos_y;
    struct g_icon *icon;
    struct wlr_scene_buffer *scene_buffer;
};

struct g_dock_panel {
    struct wl_list apps;
    struct wlr_scene_tree *scene_tree;
    struct wlr_scene_rect *background;
    struct paddings paddings;
    int apps_padding_between;
    int pos_x, pos_y;
    int width, height;
};

void g_dock_panel_create(struct tinywl_output *output);
void g_dock_panel_update(struct tinywl_output *output);
void g_dock_panel_add_application(struct tinywl_output *output, struct g_dock_app *app);
void g_dock_panel_destroy(struct tinywl_output *output);

struct g_dock_app* g_dock_app_create(
    struct tinywl_output *output, 
    const char* app_id, 
    const char* app_title, 
    const char* app_exec_command
);
void g_dock_app_destroy(struct g_dock_app *app);

// Contract
void g_dock_on_render_pass(struct g_dock_panel *panel, struct wlr_render_pass *pass);
void g_dock_app_on_render_pass(struct g_dock_app *app, struct wlr_render_pass *pass);

#endif