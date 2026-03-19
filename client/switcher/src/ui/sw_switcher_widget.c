#include "cairo.h"
#include "glib.h"
#include <stdlib.h>
#include <wayland-util.h>

#include "include/ui/sw_switcher_widget.h"
#include "gtk/gtk.h"
#include "include/ui/sw_toplevel_widget.h"
#include "include/sw_switcher.h"
#include "include/sw_toplevel.h"

#define OFFSET 120

static void draw_switcher_background(
    cairo_t *cr,
    struct sw_switcher_widget *sw
) {
    double width = sw->width;
    double height = sw->height;

    double corner_radius = sw->corner_radius;
    double aspect = 1.0; 
    double degrees = G_PI / 180.0;

    cairo_new_sub_path(cr);
    cairo_arc(cr, width - corner_radius, corner_radius, corner_radius, -90 * degrees, 0 * degrees);
    cairo_arc(cr, width - corner_radius, height - corner_radius, corner_radius, 0 * degrees, 90 * degrees);
    cairo_arc(cr, corner_radius, height - corner_radius, corner_radius, 90 * degrees, 180 * degrees);
    cairo_arc(cr, corner_radius, corner_radius, corner_radius, 180 * degrees, 270 * degrees);
    cairo_close_path(cr);

    cairo_set_source_rgba(cr, 
        26 / 255.0f, 
        26 / 255.0f, 
        26 / 255.0f, 
        1.0
    );

    cairo_fill(cr);
}

static void update_toplevel_widgets_positions(
    struct sw_switcher_widget *switcher_widget
) {
    int cx = switcher_widget->width / 2;
    int cy = switcher_widget->height / 2;

    for (int i = 0; i < PRIMATY_TOPLEVEL_COUNT; i++) {
        struct sw_toplevel *toplevel = switcher_widget->model->primary_toplevels[i];
        if (toplevel == NULL) break;

        struct sw_toplevel_widget *w = toplevel->toplevel_widget;

        if (i == INDEX_LEFT) {
            w->x = switcher_widget->inner_paddings;
            w->y = cy - w->height / 2;
            continue;
        }

        if (i == INDEX_RIGHT) {
            w->x = switcher_widget->width - switcher_widget->inner_paddings - w->width;
            w->y = cy - w->height / 2;
            continue;
        }

        if (i == INDEX_TOP) {
            w->x = cx - w->width / 2;
            w->y = switcher_widget->inner_paddings;
            continue;
        }

        if (i == INDEX_BOTTOM) {
            w->x = cx - w->width / 2;
            w->y = switcher_widget->height - switcher_widget->inner_paddings - w->height;
            continue;
        }
    }
}

static void update_toplevel_widgets_sizes(
    struct sw_switcher_widget *switcher_widget
) {
    for (int i = 0; i < PRIMATY_TOPLEVEL_COUNT; i++) {
        struct sw_toplevel *toplevel = switcher_widget->model->primary_toplevels[i];

        if (toplevel == NULL) return;

        struct sw_toplevel_widget *toplevel_widget = toplevel->toplevel_widget;
        sw_toplevel_widget_update_size(
            toplevel_widget, 
            switcher_widget->toplevel_area_width, 
            switcher_widget->toplevel_area_height
        );
    }
}

void sw_switcher_widget_init(struct sw_switcher *switcher, GtkWidget *window) {
    struct sw_switcher_widget *switcher_widget = malloc(sizeof(struct sw_switcher_widget));

    switcher_widget->model = switcher;
    switcher_widget->window = window;
    switcher_widget->width = 1;
    switcher_widget->height = 1;

    switcher->switcher_widget = switcher_widget;
}

void sw_switcher_widget_update_size(
    struct sw_switcher_widget *switcher_widget, 
    int width, int height
) {
    switcher_widget->width = width;
    switcher_widget->height = height;

    switcher_widget->inner_paddings = 16;
    switcher_widget->paddings_between_toplevels = 16;
    switcher_widget->corner_radius = 32;

    switcher_widget->content_width = switcher_widget->width 
            - switcher_widget->inner_paddings * 2;

    switcher_widget->content_height = switcher_widget->height 
            - switcher_widget->inner_paddings * 2;

    int available_toplevels_area_x = switcher_widget->content_width
            - switcher_widget->paddings_between_toplevels * 2;

    int available_toplevels_area_y = switcher_widget->content_height
            - switcher_widget->paddings_between_toplevels * 2;

    switcher_widget->toplevel_area_width = available_toplevels_area_x / 3;
    switcher_widget->toplevel_area_height = available_toplevels_area_y / 3;

    update_toplevel_widgets_sizes(switcher_widget);
    update_toplevel_widgets_positions(switcher_widget);
}

void sw_switcher_widget_on_add_toplevel(struct sw_switcher_widget *switcher_widget) {
    update_toplevel_widgets_sizes(switcher_widget);
    update_toplevel_widgets_positions(switcher_widget);

    if (switcher_widget->window) sw_switcher_widget_redraw(switcher_widget);
}

void sw_switcher_widget_on_remove_toplevel(struct sw_switcher_widget *switcher_widget) {
    update_toplevel_widgets_sizes(switcher_widget);
    update_toplevel_widgets_positions(switcher_widget);

    if (switcher_widget->window) sw_switcher_widget_redraw(switcher_widget);
}

void sw_switcher_widget_on_update_toplevel(struct sw_switcher_widget *switcher_widget) {
    update_toplevel_widgets_sizes(switcher_widget);
    update_toplevel_widgets_positions(switcher_widget);

    if (switcher_widget->window) sw_switcher_widget_redraw(switcher_widget);
}

void sw_switcher_widget_draw(
    struct sw_switcher_widget *switcher_widget,
    cairo_t *cr
) {
    draw_switcher_background(cr, switcher_widget);

    for (int i = 0; i < PRIMATY_TOPLEVEL_COUNT; i++) {
        struct sw_toplevel* toplevel = switcher_widget->model->primary_toplevels[i];
        if (toplevel == NULL) continue;

        struct sw_toplevel_widget *widget = toplevel->toplevel_widget;
        sw_toplevel_widget_draw(widget, cr);
    }
}

void sw_switcher_widget_redraw(
    struct sw_switcher_widget *switcher_widget
) {
    gtk_widget_queue_draw(switcher_widget->window);
}

void sw_switcher_widget_destroy(struct sw_switcher_widget *switcher_widget) {
    free(switcher_widget);
}