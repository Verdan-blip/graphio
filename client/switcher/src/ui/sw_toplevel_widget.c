#include <stdbool.h>
#define _POSIX_C_SOURCE 200809L

#include "glib.h"
#include "gtk/gtk.h"
#include <cairo.h>
#include <stdlib.h>
#include <string.h>

#include "include/ui/sw_toplevel_widget.h"
#include "include/ui/sw_switcher_widget.h"
#include "include/sw_toplevel.h"
#include "include/sw_switcher.h"

static const int LOAD_ICON_SIZE = 64;

static void sw_toplevel_widget_load_default_icon(struct sw_toplevel_widget *toplevel_widget) {
    GtkIconTheme *theme = gtk_icon_theme_get_default();

    toplevel_widget->pixbuff = gtk_icon_theme_load_icon(
        theme, 
        "application-x-executable", 
        LOAD_ICON_SIZE, 
        0, 
        NULL
    );
}

void sw_toplevel_widget_load_icon(struct sw_toplevel_widget *toplevel_widget) {
    GtkIconTheme *theme = gtk_icon_theme_get_default();
    const char *app_id = toplevel_widget->model->app_id;

    if (!app_id || strlen(app_id) == 0) {
        sw_toplevel_widget_load_default_icon(toplevel_widget);
        return;
    }

    GdkPixbuf *pb = NULL;

    pb = gtk_icon_theme_load_icon(theme, app_id, LOAD_ICON_SIZE, 0, NULL);

    if (!pb) {
        char *lower_id = g_utf8_strdown(app_id, -1);
        pb = gtk_icon_theme_load_icon(theme, lower_id, LOAD_ICON_SIZE, 0, NULL);
        
        if (!pb) {
            char *last_dot = strrchr(lower_id, '.');
            if (last_dot) {
                pb = gtk_icon_theme_load_icon(theme, last_dot + 1, LOAD_ICON_SIZE, 0, NULL);
            }
        }
        g_free(lower_id);
    }

    if (pb) {
        toplevel_widget->pixbuff = pb;
    } else {
        sw_toplevel_widget_load_default_icon(toplevel_widget);
    }
}

void sw_toplevel_widget_init(struct sw_toplevel *toplevel) {
    struct sw_toplevel_widget *toplevel_widget = malloc(sizeof(struct sw_toplevel_widget));
    
    toplevel_widget->model = toplevel;
    toplevel_widget->switcher_widget = toplevel->switcher->switcher_widget;
    toplevel_widget->opacity = 1.0;

    sw_toplevel_widget_load_icon(toplevel_widget);

    toplevel->toplevel_widget = toplevel_widget;
}

void sw_toplevel_widget_primary_update_size(
    struct sw_toplevel_widget *toplevel_widget, 
    int width, int height
) {
    toplevel_widget->selection_width = width;
    toplevel_widget->selection_height = height;

    toplevel_widget->width = width;
    toplevel_widget->height = height;

    toplevel_widget->selection_inner_padding = 12;
    toplevel_widget->selection_corner_radius = 24;

    toplevel_widget->icon_width = toplevel_widget->selection_width
                - toplevel_widget->selection_inner_padding * 2;

    toplevel_widget->icon_height = toplevel_widget->selection_height
                - toplevel_widget->selection_inner_padding * 2;
}

void sw_toplevel_widget_slot_update_size(
    struct sw_toplevel_widget *toplevel_widget, 
    int width, int height
) {
    toplevel_widget->selection_width = width;
    toplevel_widget->selection_height = height;

    toplevel_widget->width = width;
    toplevel_widget->height = height;

    toplevel_widget->selection_inner_padding = 12;
    toplevel_widget->selection_corner_radius = 24;

    toplevel_widget->icon_width = toplevel_widget->selection_width
                - toplevel_widget->selection_inner_padding * 2;

    toplevel_widget->icon_height = toplevel_widget->selection_height
                - toplevel_widget->selection_inner_padding * 2;
}

static void draw_rounded_rect_path(
    cairo_t *cr, 
    double x, 
    double y, 
    double w, 
    double h, 
    double r
) {
    cairo_new_sub_path(cr);
    cairo_arc(cr, x + w - r, y + r,     r, -G_PI / 2, 0);
    cairo_arc(cr, x + w - r, y + h - r, r, 0, G_PI / 2);
    cairo_arc(cr, x + r,     y + h - r, r, G_PI / 2, G_PI);
    cairo_arc(cr, x + r,     y + r,     r, G_PI, 3 * G_PI / 2);
    cairo_close_path(cr);
}

void sw_toplevel_widget_draw(
    struct sw_toplevel_widget *toplevel_widget,
    struct sw_switcher_widget *switcher_widget, 
    cairo_t *cr
) {
    int real_icon_width = gdk_pixbuf_get_width(toplevel_widget->pixbuff);
    int real_icon_height = gdk_pixbuf_get_height(toplevel_widget->pixbuff);

    int icon_padding_x = (toplevel_widget->selection_width - real_icon_width) / 2;
    int icon_padding_y = (toplevel_widget->selection_height - real_icon_height) / 2;

    double scale_x = toplevel_widget->icon_width * 1.0 / real_icon_width;
    double scale_y = toplevel_widget->icon_height * 1.0 / real_icon_height;

    double scale_offset_x = (real_icon_width - toplevel_widget->icon_width) / 2.0;
    double scale_offset_y = (real_icon_height - toplevel_widget->icon_height) / 2.0;

    cairo_save(cr);

    cairo_translate(
        cr, 
        toplevel_widget->x + icon_padding_x + scale_offset_x, 
        toplevel_widget->y + icon_padding_y + scale_offset_y
    );

    cairo_scale(cr, scale_x, scale_y);

    gdk_cairo_set_source_pixbuf(
        cr, 
        toplevel_widget->pixbuff, 
        0,
        0
    );

    cairo_paint_with_alpha(cr, toplevel_widget->opacity);

    cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_BILINEAR);

    cairo_paint(cr);
    cairo_restore(cr);

    draw_rounded_rect_path(
        cr,
        toplevel_widget->x, 
        toplevel_widget->y, 
        toplevel_widget->selection_width, 
        toplevel_widget->selection_height, 
        toplevel_widget->selection_corner_radius
    );

    if (toplevel_widget == switcher_widget->selected_toplevel_widget) {
        cairo_set_source_rgba(
            cr, 
            173 / 255.0f, 
            173 / 255.0f, 
            255 / 255.0f, 
            255 / 255.0f
        );
    } else if (toplevel_widget == switcher_widget->model->current_toplevel->toplevel_widget) {
        cairo_set_source_rgba(
            cr, 
            173 / 255.0f, 
            173 / 255.0f, 
            255 / 255.0f, 
            128 / 255.0f
        );
    } else {
        cairo_set_source_rgba(
            cr, 
            173 / 255.0f, 
            173 / 255.0f, 
            255 / 255.0f, 
            25 / 255.0f
        );
    }
    
    cairo_set_line_width(cr, 2.0);
    cairo_stroke(cr);
}

void sw_toplevel_widget_draw_placeholder(
    int x, int y, 
    int w, int h,
    int corner_radius,
    cairo_t *cr
) {
    double dashes[] = { 10.0, 10.0 }; 
    int num_dashes = 2;
    double offset = 0;

    cairo_set_source_rgba(
        cr, 
        173 / 255.0f, 
        173 / 255.0f, 
        255 / 255.0f, 
        128 / 255.0f
    );

    cairo_set_dash(cr, dashes, num_dashes, offset);

    draw_rounded_rect_path(
        cr,
        x, 
        y, 
        w, 
        h, 
        corner_radius
    );

    cairo_set_line_width(cr, 2.0);
    cairo_stroke(cr);

    cairo_set_dash(cr, NULL, 0, 0);
}

void sw_toplevel_widget_draw_selection(
    int x, int y, 
    int w, int h,
    int corner_radius,
    cairo_t *cr
) {
    cairo_set_source_rgba(
        cr, 
        173 / 255.0f, 
        173 / 255.0f, 
        255 / 255.0f, 
        255 / 255.0f
    );

    draw_rounded_rect_path(
        cr,
        x, 
        y, 
        w, 
        h, 
        corner_radius
    );

    cairo_set_line_width(cr, 2.0);
    cairo_stroke(cr);
}

void sw_toplevel_widget_destroy(struct sw_toplevel_widget *tw) {
    if (tw->pixbuff) g_object_unref(tw->pixbuff);
    free(tw);
}