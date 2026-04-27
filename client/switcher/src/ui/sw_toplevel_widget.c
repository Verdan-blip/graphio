#include "include/math/sw_color.h"
#include "include/math/sw_vec2.h"
#include "include/ui/sw_primitives.h"
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
    const char *app_id = toplevel_widget->toplevel->app_id;

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
    
    toplevel_widget->toplevel = toplevel;
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

    toplevel_widget->selection_inner_padding = 12;
    toplevel_widget->selection_corner_radius = 24;
}

void sw_toplevel_widget_slot_update_size(
    struct sw_toplevel_widget *toplevel_widget, 
    int width, int height
) {
    toplevel_widget->selection_width = width;
    toplevel_widget->selection_height = height;

    toplevel_widget->selection_inner_padding = 12;
    toplevel_widget->selection_corner_radius = 24;
}

void sw_toplevel_widget_draw(
    struct sw_toplevel_widget *toplevel_widget,
    struct sw_switcher_widget *switcher_widget, 
    struct sw_vec2 pos,
    struct sw_vec2 size,
    cairo_t *cr
) {
    if (size.x <= 0 || size.y <= 0) return;

    int real_icon_width = gdk_pixbuf_get_width(toplevel_widget->pixbuff);
    int real_icon_height = gdk_pixbuf_get_height(toplevel_widget->pixbuff);

    double scale_x = size.x / real_icon_width;
    double scale_y = size.y / real_icon_height;

    double scale_offset_x = -(real_icon_width - size.x) / 2.0;
    double scale_offset_y = -(real_icon_height - size.y) / 2.0;

    cairo_save(cr);

    cairo_translate(cr, pos.x, pos.y);
    cairo_scale(cr, scale_x, scale_y);

    gdk_cairo_set_source_pixbuf(cr,  toplevel_widget->pixbuff, 0, 0);

    cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_BILINEAR);
    cairo_paint_with_alpha(cr, toplevel_widget->opacity);

    cairo_paint(cr);
    cairo_restore(cr);
}

void sw_toplevel_widget_draw_placeholder(
    struct sw_vec2 pos,
    struct sw_vec2 size,
    int corner_radius,
    struct sw_color color,
    cairo_t *cr
) {
    sw_draw_dashed_round_corner_rect(
        pos, 
        size, 
        corner_radius, 
        2, 
        color, 
        cr
    );
}

void sw_toplevel_widget_destroy(struct sw_toplevel_widget *tw) {
    if (tw->pixbuff) g_object_unref(tw->pixbuff);
    free(tw);
}