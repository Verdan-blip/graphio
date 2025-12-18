#ifndef G_DOCK_APP_H
#define G_DOCK_APP_H

#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_pointer.h>

struct g_output;

struct g_dock_app {
    struct wl_list link;
    char *id;
    char *title;
    char* cmd;
    int pos_x, pos_y;
    struct g_icon *icon;
};

struct g_dock_app* g_dock_app_create(
    struct g_output *output, 
    const char* app_id, 
    const char* app_title, 
    const char* app_exec_command
);

void g_dock_app_destroy(struct g_dock_app *app);

//Launch
void g_dock_app_launch(const struct g_dock_app *app);

// Contract
void g_dock_app_on_render_pass(struct g_dock_app *app, struct wlr_render_pass *pass);

bool g_dock_app_consume_cursor_button_event(
    struct g_dock_app *app, 
    double x, 
    double y, 
    struct wlr_pointer_button_event *event
);

#endif