#ifndef SW_SWITCHER_WIDGET
#define SW_SWITCHER_WIDGET

#include "cairo.h"
#include "gtk/gtk.h"

struct sw_switcher;
struct sw_toplevel_widget;

struct sw_switcher_widget {
    struct sw_switcher *model;

    int width, height;
    int corner_radius;
    
    int inner_paddings;
    int paddings_between_toplevels;

    int content_width, content_height;
    int toplevel_area_width, toplevel_area_height;

    struct sw_toplevel_widget *active_toplevel_widget;

    GtkWidget *window;
};

void sw_switcher_widget_init(struct sw_switcher *switcher, GtkWidget *window);

void sw_switcher_widget_update_size(
    struct sw_switcher_widget *switcher_widget, 
    int width, int height
);

void sw_switcher_widget_draw(
    struct sw_switcher_widget *switcher_widget,
    cairo_t *cr
);

void sw_switcher_widget_redraw(
    struct sw_switcher_widget *switcher_widget
);

void sw_switcher_widget_on_add_toplevel(
    struct sw_switcher_widget *switcher_widget,
    struct sw_toplevel_widget *toplevel_widget
);

void sw_switcher_widget_on_remove_toplevel(
    struct sw_switcher_widget *switcher_widget,
    struct sw_toplevel_widget *toplevel_widget
);

void sw_switcher_widget_on_update_toplevel(
    struct sw_switcher_widget *switcher_widget
);

void sw_switcher_widget_mark_toplevel_activated(
    struct sw_switcher_widget *switcher_widget,
    struct sw_toplevel_widget *toplevel_widget
);

void sw_switcher_widget_destroy(struct sw_switcher_widget *switcher_widget);

#endif