#ifndef SW_SWITCHER_WIDGET
#define SW_SWITCHER_WIDGET

#include "cairo.h"
#include "gtk/gtk.h"
#include "include/math/sw_color.h"
#include "include/math/sw_vec2.h"
#include "include/sw_switcher.h"
#include "include/ui/animation/sw_animation_manager.h"
#include "include/ui/sw_border_item.h"
#include "include/ui/sw_flow_layout.h"
#include "include/ui/sw_polar_layout.h"
#include <stdbool.h>

struct sw_switcher;
struct sw_toplevel_widget;

enum sw_switcher_widget_current_zone {
    SW_SWITCHER_PRIMARY_ZONE,
    SW_SWITCHER_PRIMARY_ZONE_BORDERING,
    SW_SWITCHER_SLOT_ZONE
};

struct sw_switcher_widget {
    struct sw_switcher *switcher;

    struct sw_polar_layout *primary_layout;
    struct sw_flow_layout *slot_layout;

    struct sw_vec2 primary_layout_pos;
    struct sw_vec2 slot_layout_pos;

    struct sw_border_item *selection_border;
    struct sw_border_item *current_toplevel_border;

    struct sw_vec2 size;

    int padding_between_layouts;

    struct sw_toplevel_widget *selected_toplevel_widget;
    enum sw_switcher_widget_current_zone current_zone;

    struct sw_color primary_bg_active_color;
    struct sw_color primary_bg_inactive_color;
    struct sw_color slot_bg_active_color;
    struct sw_color slot_bg_inactive_color;

    struct sw_color selection_color;
    struct sw_color current_indicator_color;

    struct sw_animation_manager *animation_manager;

    bool is_slot_widget_visible;
    GtkWidget *window;
};

void sw_switcher_widget_init(struct sw_switcher *switcher, GtkWidget *window);

void sw_switcher_widget_update_size(
    struct sw_switcher_widget *switcher_widget, 
    struct sw_vec2 size
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

void sw_switcher_widget_mark_toplevel_selected(
    struct sw_switcher_widget *switcher_widget,
    struct sw_toplevel_widget *toplevel_widget
);

void sw_switcher_widget_notify_is_current_toplevel_change(
    struct sw_switcher_widget *switcher_widget,
    struct sw_toplevel_widget *toplevel_widget
);

void sw_switcher_widget_destroy(struct sw_switcher_widget *switcher_widget);

void sw_switcher_widget_enter_primary_zone(struct sw_switcher_widget *switcher_widget);
void sw_switcher_widget_enter_slot_zone(struct sw_switcher_widget *switcher_widget);

void sw_switcher_widget_notify_topology_changed(struct sw_switcher_widget *switcher_widget);

void sw_switcher_widget_notify_positions_changed(struct sw_switcher_widget *switcher_widget);
void sw_switcher_widget_notify_sizes_changed(struct sw_switcher_widget *switcher_widget);

#endif