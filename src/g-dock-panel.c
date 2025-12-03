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
#include "../include/g-dock-app.h"
#include "../include/g-dock-panel.h"
#include "../include/g-server.h"
#include "../include/g-output.h"

struct g_dock_panel* g_dock_panel_create(struct g_output *output) {
    struct g_dock_panel *dock_panel = calloc(1, sizeof(struct g_dock_panel));
    dock_panel->output = output;

    float dock_panel_bg_color[4] = {0.1f, 0.1f, 0.1f, 0.8f};

    dock_panel->paddings = create_paddings_all(16);

    dock_panel->pos_x = 0;
    dock_panel->pos_y = 0;
    dock_panel->width = 0;
    dock_panel->height = 0;
    dock_panel->apps_padding_between = 16;

    wl_list_init(&dock_panel->apps);

    return dock_panel;
}

void g_dock_panel_update(struct g_dock_panel *panel) {
    int app_pos_x = 0;
    int app_pos_y = panel->paddings.top;

    int app_count = wl_list_length(&panel->apps);

    int app_max_height = 0;
    int app_icon_width_sum = 0;

    struct g_dock_app *app;
    int i = 0;
    wl_list_for_each(app, &panel->apps, link) {
        app_pos_x = i * (app->icon->width + panel->apps_padding_between) + panel->paddings.start;

        app_max_height = max(app_max_height, app->icon->height);
        app_icon_width_sum += app->icon->width;

        app->pos_x = app_pos_x;
        app->pos_y = app_pos_y;

        i++;
    }

    panel->width = panel->paddings.start + 
        app_icon_width_sum + 
        (app_count - 1) * panel->apps_padding_between + 
        panel->paddings.end;

    panel->height = app_max_height + panel->paddings.start + panel->paddings.end;

    struct wlr_box output_bounding_box;
    wlr_output_layout_get_box(
        panel->output->server->output_layout,
        panel->output->wlr_output,
        &output_bounding_box
    );

    panel->pos_x = output_bounding_box.x + (output_bounding_box.width - panel->width) / 2;
    panel->pos_y = output_bounding_box.y + output_bounding_box.height - panel->height - panel->paddings.bottom;

    // Offset app positions
    wl_list_for_each(app, &panel->apps, link) {
        app->pos_x += panel->pos_x;
        app->pos_y += panel->pos_y;
    }
}

void g_dock_panel_add_app(struct g_dock_panel *panel, struct g_dock_app *app) {
    wl_list_insert(&panel->apps, &app->link);
    g_dock_panel_update(panel);
}

void g_dock_panel_destroy(struct g_dock_panel *panel) {
    struct g_dock_app *app;

    wl_list_for_each(app, &panel->apps, link) {
        g_dock_app_destroy(app);
    }

    free(panel);
}

// Contract
void g_dock_panel_on_render_pass(struct g_dock_panel *panel, struct wlr_render_pass *pass) {
    wlr_render_pass_add_rect(pass, &(struct wlr_render_rect_options) {
        .box = { panel->pos_x, panel->pos_y, panel->width, panel->height },
        .color = { 0.1f, 0.1f, 0.1f, 0.8f }
    });

    struct g_dock_app *app;
	wl_list_for_each(app, &panel->apps, link) {
		g_dock_app_on_render_pass(app, pass);
	}
}

bool g_dock_panel_consume_cursor_button_event(
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