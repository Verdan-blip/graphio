#define _POSIX_C_SOURCE 200809L

#include "glib.h"
#include "gtk/gtk.h"
#include <cairo.h>
#include <stdlib.h>
#include <string.h>

#include "include/ui/sw_toplevel_widget.h"
#include "include/sw_toplevel.h"
#include "include/sw_switcher.h"

static const int LOAD_ICON_SIZE = 64;

static GdkPixbuf* load_best_icon(const char* app_id, int size) {
    GtkIconTheme *theme = gtk_icon_theme_get_default();
    if (!app_id) return gtk_icon_theme_load_icon(theme, "application-x-executable", size, 0, NULL);

    char *id_copy = strdup(app_id);
    char *current = id_copy;
    GdkPixbuf *pb = NULL;

    while (current) {
        pb = gtk_icon_theme_load_icon(theme, current, size, GTK_ICON_LOOKUP_FORCE_SIZE, NULL);
        if (pb) break;
        current = strchr(current, '.');
        if (current) current++;
    }
    free(id_copy);
    return pb ? pb : gtk_icon_theme_load_icon(theme, "application-x-executable", size, 0, NULL);
}

void sw_toplevel_widget_init(struct sw_toplevel *model) {
    struct sw_toplevel_widget *toplevel_widget = malloc(sizeof(struct sw_toplevel_widget));
    
    toplevel_widget->model = model;
    toplevel_widget->switcher_widget = model->switcher->switcher_widget;
    toplevel_widget->opacity = 1.0;
    toplevel_widget->pixbuff = load_best_icon(model->app_id, LOAD_ICON_SIZE);

    model->toplevel_widget = toplevel_widget;
}

void sw_toplevel_widget_update_size(struct sw_toplevel_widget *toplevel_widget, int width, int height) {
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
    cairo_t *cr
) {
    int real_icon_width = gdk_pixbuf_get_width(toplevel_widget->pixbuff);
    int real_icon_height = gdk_pixbuf_get_height(toplevel_widget->pixbuff);

    int icon_padding_x = (toplevel_widget->selection_width - real_icon_width) / 2;
    int icon_padding_y = (toplevel_widget->selection_height - real_icon_height) / 2;

    gdk_cairo_set_source_pixbuf(
        cr, 
        toplevel_widget->pixbuff, 
        toplevel_widget->x + icon_padding_x, 
        toplevel_widget->y + icon_padding_y
    );

    cairo_paint_with_alpha(cr, toplevel_widget->opacity);

    draw_rounded_rect_path(
        cr,
        toplevel_widget->x, 
        toplevel_widget->y, 
        toplevel_widget->selection_width, 
        toplevel_widget->selection_height, 
        toplevel_widget->selection_corner_radius
    );

    if (toplevel_widget->model->activated) {
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

void sw_toplevel_widget_destroy(struct sw_toplevel_widget *tw) {
    if (tw->pixbuff) g_object_unref(tw->pixbuff);
    free(tw);
}