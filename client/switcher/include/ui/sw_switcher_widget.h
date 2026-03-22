#ifndef SW_SWITCHER_WIDGET
#define SW_SWITCHER_WIDGET

#include "cairo.h"
#include "gtk/gtk.h"
#include <stdbool.h>

struct sw_switcher;
struct sw_toplevel_widget;

enum sw_switcher_widget_current_zone {
    SW_SWITCHER_PRIMARY_ZONE,
    SW_SWITCHER_PRIMARY_ZONE_BORDERING,
    SW_SWITCHER_SLOT_ZONE
};

struct sw_switcher_widget {
    struct sw_switcher *model;

    int width, height;
    int corner_radius;

    int primary_square_width, primary_square_height;
    int slot_rect_width, slot_rect_height; 
    int padding_between_containers;
    
    int primary_square_horizontal_padding;
    
    int inner_paddings;
    int paddings_between_toplevels;

    int content_width, content_height;
    int toplevel_primary_area_width, toplevel_primary_area_height;
    int toplevel_slot_area_width, toplevel_slot_area_height;

    struct sw_toplevel_widget *selected_toplevel_widget;

    enum sw_switcher_widget_current_zone current_zone;

    bool is_slot_widget_visible;

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

void sw_switcher_widget_mark_toplevel_selected(
    struct sw_switcher_widget *switcher_widget,
    struct sw_toplevel_widget *toplevel_widget
);

void sw_switcher_widget_enter_primary_zone(struct sw_switcher_widget *switcher_widget);

void sw_switcher_widget_enter_slot_zone(struct sw_switcher_widget *switcher_widget);

void sw_switcher_widget_destroy(struct sw_switcher_widget *switcher_widget);

#endif