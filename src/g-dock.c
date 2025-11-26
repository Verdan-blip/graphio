#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wlr/util/log.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>

#include "../include/utils.h"
#include "../include/g-dock.h"
#include "../include/g-server.h"

void g_dock_panel_create(struct tinywl_output *output) {
    struct tinywl_server *server = output->server;
    struct g_dock_panel *dock_panel = calloc(1, sizeof(struct g_dock_panel));

    float dock_panel_bg_color[4] = {0.1f, 0.1f, 0.1f, 0.8f};

    dock_panel->paddings = create_paddings_all(16);

    dock_panel->scene_tree = wlr_scene_tree_create(&server->scene->tree);

    dock_panel->pos_x = 0;
    dock_panel->pos_y = 0;
    dock_panel->width = 0;
    dock_panel->height = 0;
    dock_panel->apps_padding_between = 16;

    dock_panel->background = wlr_scene_rect_create(
        dock_panel->scene_tree, 
        dock_panel->width, 
        dock_panel->height, 
        dock_panel_bg_color
    );

    wlr_scene_node_set_position(
        &dock_panel->background->node,
        dock_panel->pos_x, 
        dock_panel->pos_y
    );

    wl_list_init(&dock_panel->apps);

    output->dock_panel = dock_panel;
}

void g_dock_panel_update(struct tinywl_output *output) {
    struct g_dock_panel *dock_panel = output->dock_panel;

    int app_pos_x = 0;
    int app_pos_y = dock_panel->paddings.top;

    int app_count = wl_list_length(&dock_panel->apps);

    int app_max_height = 0;
    int app_icon_width_sum = 0;

    struct g_dock_app *app;
    int i = 0;
    wl_list_for_each(app, &dock_panel->apps, link) {
        app_pos_x = i * (app->icon->width + dock_panel->apps_padding_between) + dock_panel->paddings.start;

        app_max_height = max(app_max_height, app->icon->height);
        app_icon_width_sum += app->icon->width;

        app->pos_x = app_pos_x;
        app->pos_y = app_pos_y;

        i++;
    }

    dock_panel->width = dock_panel->paddings.start + 
        app_icon_width_sum + 
        (app_count - 1) * dock_panel->apps_padding_between + 
        dock_panel->paddings.end;

    dock_panel->height = app_max_height + dock_panel->paddings.start + dock_panel->paddings.end;

    struct wlr_box output_bounding_box;
    wlr_output_layout_get_box(
        output->server->output_layout,
        output->wlr_output,
        &output_bounding_box
    );

    dock_panel->pos_x = output_bounding_box.x + (output_bounding_box.width - dock_panel->width) / 2;
    dock_panel->pos_y = output_bounding_box.y + output_bounding_box.height - dock_panel->height - dock_panel->paddings.bottom;

    // Offset app positions
    wl_list_for_each(app, &dock_panel->apps, link) {
        app->pos_x += dock_panel->pos_x;
        app->pos_y += dock_panel->pos_y;
    }

    wlr_scene_rect_set_size(dock_panel->background, dock_panel->width, dock_panel->height);
    wlr_scene_node_set_position(&dock_panel->background->node, dock_panel->pos_x, dock_panel->pos_y);
}

void g_dock_panel_add_application(struct tinywl_output *output, struct g_dock_app *app) {
    wl_list_insert(&output->dock_panel->apps, &app->link);
    g_dock_panel_update(output);
}

void g_dock_panel_destroy(struct tinywl_output *output) {
    struct g_dock_app *app;

    wl_list_for_each(app, &output->dock_panel->apps, link) {
        g_dock_app_destroy(app);
    }

    free(&output->dock_panel->apps);

    wlr_scene_node_destroy(&output->dock_panel->background->node);
}

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
    struct tinywl_output *output,
    const char *app_id, 
    const char *app_title, 
    const char *app_exec_command
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

// Contract
void g_dock_app_on_render_pass(struct g_dock_app *app, struct wlr_render_pass *pass) {
    g_icon_on_render_pass(app->icon, app->pos_x, app->pos_y, pass);
}

void g_dock_on_render_pass(struct g_dock_panel *panel, struct wlr_render_pass *pass) {
    struct g_dock_app *app;
	wl_list_for_each(app, &panel->apps, link) {
		g_dock_app_on_render_pass(app, pass);
	}
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

bool g_dock_consume_cursor_button_event(
    struct g_dock_panel *panel, 
    double x, 
    double y, 
    struct wlr_pointer_button_event *event
) {
    struct g_dock_app *app;
    wl_list_for_each(app, &panel->apps, link) {
        if (g_dock_app_consume_cursor_button_event(app, x, y, event)) {
            return true;
        }
    }
    return false;
}

// Launch
void g_dock_app_launch(const struct g_dock_app *app) {
    if (fork() == 0) {
		execl("/bin/sh", "/bin/sh", "-c", app->cmd, (void *)NULL);
	}
}