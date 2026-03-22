#ifndef SW_TOPLEVEL_WIDGET
#define SW_TOPLEVEL_WIDGET

#include "cairo.h"
#include "gdk-pixbuf/gdk-pixbuf.h"
#include <stdbool.h>

struct sw_toplevel;
struct sw_switcher_widget;

struct sw_toplevel_widget {
    struct sw_switcher_widget *switcher_widget;

    GdkPixbuf *pixbuff;
    
    int x, y;
    float opacity;

    int width, height;
    int icon_width, icon_height;
    int selection_width, selection_height;
    int selection_inner_padding;
    int selection_corner_radius;

    struct sw_toplevel *model;
};

void sw_toplevel_widget_init(struct sw_toplevel *toplevel);

void sw_toplevel_widget_load_icon(struct sw_toplevel_widget *toplevel_widget);

void sw_toplevel_widget_primary_update_size(
    struct sw_toplevel_widget *toplevel_widget, 
    int width, int height
);

void sw_toplevel_widget_slot_update_size(
    struct sw_toplevel_widget *toplevel_widget, 
    int width, int height
);

void sw_toplevel_widget_draw(
    struct sw_toplevel_widget *toplevel_widget, 
    struct sw_switcher_widget *switcher_widget,
    cairo_t *cr
);

void sw_toplevel_widget_draw_placeholder(
    int x, 
    int y, 
    int w, 
    int h, 
    int corner_radius, 
    cairo_t *cr
);

void sw_toplevel_widget_draw_selection(
    int x, int y, 
    int w, int h,
    int corner_radius,
    cairo_t *cr
);

void sw_toplevel_widget_destroy(struct sw_toplevel_widget *toplevel_widget);

#endif