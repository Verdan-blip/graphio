#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/util/log.h>
#include "../include/g-dock-app.h"
#include "../include/g-output.h"
#include "../include/g-icon.h"

static char* g_icon_path_find_by_app_id(const char *app_id) {
    char path[1024];

    const char *hicolor_dir_path = "/usr/share/icons/hicolor/48x48/apps/%s.png";

    snprintf(path, sizeof(path), hicolor_dir_path, app_id);

    if (access(path, F_OK) == 0) {
        return strdup(path);
    }

    wlr_log(WLR_DEBUG, "No icon found for app_id: %s", app_id);

    return NULL;
}

struct g_dock_app* g_dock_app_create(
    struct g_output *output, 
    const char* app_id, 
    const char* app_title, 
    const char* app_exec_command
) {
    struct g_dock_app *app = calloc(1, sizeof(struct g_dock_app));
    app->id = strdup(app_id),
    app->title = strdup(app_title);
    app->cmd = strdup(app_exec_command);
    
    char* icon_path = g_icon_path_find_by_app_id(app_id);

    app->icon = g_icon_load_from_path_or_default(icon_path, 48, 48, output->server->renderer);

    return app;
}

void g_dock_app_destroy(struct g_dock_app *app) {
    wl_list_remove(&app->link);
    g_icon_destroy(app->icon);
    free(app);
}

bool g_dock_app_consume_cursor_button_event(
    struct g_dock_app *app, 
    double x, 
    double y, 
    struct wlr_pointer_button_event *event
) {
    if (event->state == WL_POINTER_BUTTON_STATE_PRESSED) {
        if (x >= app->pos_x && 
            x <= app->pos_x + app->icon->width &&
            y >= app->pos_y && 
            y <= app->pos_y + app->icon->height
        ) {
            g_dock_app_launch(app);
            return true;
        }
    }
    return false;
}

//Launch
void g_dock_app_launch(const struct g_dock_app *app) {
if (fork() == 0) {
		execl("/bin/sh", "/bin/sh", "-c", app->cmd, (void *)NULL);
	}
}

// Render contract
void g_dock_app_on_render_pass(struct g_dock_app *app, struct wlr_render_pass *pass) {
    g_icon_on_render_pass(app->icon, app->pos_x, app->pos_y, pass);
}