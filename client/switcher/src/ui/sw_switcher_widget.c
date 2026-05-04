#include "cairo.h"
#include <stdbool.h>
#include <stdlib.h>
#include <wayland-util.h>

#include "include/ui/sw_switcher_widget.h"
#include "include/math/sw_color.h"
#include "include/math/sw_vec2.h"
#include "include/ui/animation/sw_animation.h"
#include "include/ui/animation/sw_animation_manager.h"
#include "include/ui/animation/sw_easing.h"
#include "include/ui/sw_border_item.h"
#include "include/ui/sw_flow_layout.h"
#include "include/ui/sw_polar_layout.h"
#include "gtk/gtk.h"
#include "include/graph/sw_graph_model.h"
#include "include/ui/sw_primitives.h"
#include "include/ui/sw_toplevel_widget.h"
#include "include/sw_switcher.h"
#include "include/sw_toplevel.h"

#define SLOT_H_RATIO 0.2
#define GAP_H_RATIO 0.05
#define PRIMARY_H_RATIO 0.75

#define RADIUS_RATIO 0.07
#define INNER_PAD_RATIO 0.04

void sw_switcher_widget_notify_topology_changed(struct sw_switcher_widget *switcher_widget) {
    struct sw_graph_model *graph = switcher_widget->switcher->graph_model;

    struct sw_polar_layout *primary_layout = switcher_widget->primary_layout;
    struct sw_flow_layout *slot_layout = switcher_widget->slot_layout;

    sw_polar_layout_set_item_data(
        primary_layout, 
        SW_POLAR_ITEM_POSITION_NORTH, 
        graph->north_node ? graph->north_node->data : NULL
    );

    sw_polar_layout_set_item_data(
        primary_layout, 
        SW_POLAR_ITEM_POSITION_SOUTH, 
        graph->south_node ? graph->south_node->data : NULL
    );

    sw_polar_layout_set_item_data(
        primary_layout, 
        SW_POLAR_ITEM_POSITION_WEST, 
        graph->west_node ? graph->west_node->data : NULL
    );

    sw_polar_layout_set_item_data(
        primary_layout, 
        SW_POLAR_ITEM_POSITION_EAST, 
        graph->east_node ? graph->east_node->data : NULL
    );

    sw_flow_layout_clear(slot_layout);

    struct sw_graph_node *node;
    sw_graph_model_for_each_slot(node, graph) {
        sw_flow_layout_add_item(slot_layout, node->data);
    }

    sw_switcher_widget_redraw(switcher_widget);
}

void sw_switcher_widget_init(struct sw_switcher *switcher, GtkWidget *window) {
    struct sw_switcher_widget *switcher_widget = calloc(1, sizeof(struct sw_switcher_widget));

    switcher_widget->switcher = switcher;
    switcher_widget->window = window;
    switcher_widget->size = sw_vec2_create(1, 1);

    switcher_widget->primary_layout = sw_polar_layout_create();
    switcher_widget->slot_layout = sw_flow_layout_create();

    switcher_widget->selection_border = sw_border_item_create();
    switcher_widget->current_toplevel_border = sw_border_item_create();

    switcher_widget->is_slot_widget_visible = false;

    switcher_widget->current_zone = SW_SWITCHER_PRIMARY_ZONE;

    switcher_widget->selected_toplevel_widget = NULL;

    switcher_widget->primary_bg_active_color = sw_color_create(0.10, 0.11, 0.15, 0.85);
    switcher_widget->primary_bg_inactive_color = sw_color_create(0.10, 0.11, 0.15, 0.55);
    
    switcher_widget->slot_bg_active_color  = sw_color_create(0.10, 0.11, 0.15, 0.85);
    switcher_widget->slot_bg_inactive_color  = sw_color_create(0.10, 0.11, 0.15, 0.55);

    switcher_widget->current_indicator_color = sw_color_create(0.68f, 0.68f, 1.0f, 0.5f);
    switcher_widget->selection_color = sw_color_create(0.68f, 0.68f, 1.0f, 1.0f);

    switcher_widget->animation_manager = sw_animation_manager_create();

    switcher->switcher_widget = switcher_widget;
}

void sw_switcher_widget_update_size(
    struct sw_switcher_widget *switcher_widget, 
    struct sw_vec2 size
) {
    switcher_widget->size = size;

    double slot_height = size.y * SLOT_H_RATIO;
    struct sw_vec2 primary_size = sw_vec2_create(size.y * PRIMARY_H_RATIO, size.y * PRIMARY_H_RATIO);
    double gap = size.y - slot_height - primary_size.y;

    double slot_inner_padding = 24;
    double slot_gap_between_items = 32;
    double slot_item_height = slot_height - slot_inner_padding * 2;

    sw_flow_layout_resize(
        switcher_widget->slot_layout,
        size.x,
        sw_vec2_create(slot_item_height, slot_item_height),
        slot_inner_padding,
        slot_gap_between_items
    );

    double primary_inner_padding = 24;
    sw_polar_layout_resize(
        switcher_widget->primary_layout,
        primary_size, 
        primary_inner_padding
    );

    switcher_widget->primary_layout_pos.x = (size.x - switcher_widget->primary_layout->canvas_size.x) / 2;
    switcher_widget->primary_layout_pos.y = slot_height + gap;

    switcher_widget->slot_layout_pos.x = 0;
    switcher_widget->slot_layout_pos.y = 0;

    switcher_widget->padding_between_layouts = gap;

    struct sw_polar_layout_item *item = switcher_widget->primary_layout->center;
    struct sw_vec2 selection_pos = sw_vec2_add(item->pos, switcher_widget->primary_layout_pos);
    struct sw_vec2 selection_size = item->size;
    double corner_radius = item->corner_radius;

    sw_border_item_set_thickness(switcher_widget->selection_border, 3.0);
    sw_border_item_set_size(switcher_widget->selection_border, selection_size);
    sw_border_item_set_corner_radius(switcher_widget->selection_border, corner_radius);
    sw_border_item_set_position(switcher_widget->selection_border, selection_pos);
}

void sw_switcher_widget_on_add_toplevel(
    struct sw_switcher_widget *switcher_widget,
    struct sw_toplevel_widget *toplevel
) {
    struct sw_graph_model *graph = switcher_widget->switcher->graph_model;

    if (sw_graph_model_slot_nodes_empty(graph)) {
        switcher_widget->is_slot_widget_visible = false;
    } else {
        switcher_widget->is_slot_widget_visible = true;
    }

    sw_switcher_widget_notify_topology_changed(switcher_widget);

    if (switcher_widget->window) sw_switcher_widget_redraw(switcher_widget);
}

void sw_switcher_widget_on_remove_toplevel(
    struct sw_switcher_widget *switcher_widget,
    struct sw_toplevel_widget *toplevel_widget
) {
    struct sw_graph_model *graph = switcher_widget->switcher->graph_model;

    if (sw_graph_model_slot_nodes_empty(graph)) {
        switcher_widget->is_slot_widget_visible = false;
    } else {
        switcher_widget->is_slot_widget_visible = true;
    }

    sw_switcher_widget_notify_topology_changed(switcher_widget);

    if (switcher_widget->selected_toplevel_widget == toplevel_widget) {
        switcher_widget->selected_toplevel_widget = NULL;
    }

    if (switcher_widget->window) sw_switcher_widget_redraw(switcher_widget);
}

static void draw_slot_zone(struct sw_switcher_widget *sw, cairo_t *cr, bool active) {
    if (!sw->is_slot_widget_visible) return;

    struct sw_flow_layout *layout = sw->slot_layout;
    struct sw_color bg_color = active ? sw->slot_bg_active_color : sw->slot_bg_inactive_color;
    float opacity = active ? 1.0f : 0.5f;

    sw_draw_filled_round_corner_rect(
        sw->slot_layout_pos,
        sw_vec2_create(layout->max_width, layout->total_height),
        layout->corner_radius, bg_color, cr
    );

    for (int i = 0; i < layout->items_count; i++) {
        struct sw_flow_item *item = layout->items[i];
        struct sw_toplevel *toplevel = item->data;
        
        toplevel->toplevel_widget->opacity = opacity;
        sw_toplevel_widget_draw(
            toplevel->toplevel_widget, sw,
            sw_vec2_add(item->pos, sw->slot_layout_pos),
            item->size, cr
        );
    }
}

static void draw_primary_zone(struct sw_switcher_widget *sw, cairo_t *cr, bool active) {
    struct sw_polar_layout *layout = sw->primary_layout;
    struct sw_color bg_color = active ? sw->slot_bg_active_color : sw->slot_bg_inactive_color;
    float opacity = active ? 1.0f : 0.25f;

    sw_draw_filled_round_corner_rect(
        sw->primary_layout_pos, layout->canvas_size,
        layout->corner_radius, bg_color, cr
    );

    struct sw_polar_layout_item *items[] = {layout->west, layout->east, layout->north, layout->south};

    for (int i = 0; i < 4; i++) {
        struct sw_vec2 pos = sw_vec2_add(items[i]->pos, sw->primary_layout_pos);
        
        if (!items[i]->data) {
            sw_toplevel_widget_draw_placeholder(pos, items[i]->size, items[i]->corner_radius, sw->current_indicator_color, cr);
        } else {
            struct sw_toplevel *toplevel = items[i]->data;
            toplevel->toplevel_widget->opacity = opacity;
            sw_toplevel_widget_draw(toplevel->toplevel_widget, sw, pos, items[i]->size, cr);
        }
    }
}

void sw_switcher_widget_draw(struct sw_switcher_widget *sw, cairo_t *cr) {
    bool primary_active = (sw->current_zone == SW_SWITCHER_PRIMARY_ZONE);

    if (primary_active) {
        draw_slot_zone(sw, cr, false);
        draw_primary_zone(sw, cr, true);
    } else {
        draw_primary_zone(sw, cr, false);
        draw_slot_zone(sw, cr, true);
    }

    sw_draw_outlined_round_corner_rect(
        sw->current_toplevel_border->pos, sw->current_toplevel_border->size,
        sw->current_toplevel_border->corner_radius, sw->current_toplevel_border->thickness,
        sw->current_indicator_color, cr
    );

    sw_draw_outlined_round_corner_rect(
        sw->selection_border->pos, sw->selection_border->size,
        sw->selection_border->corner_radius, sw->selection_border->thickness,
        sw->selection_color, cr
    );
}

void sw_switcher_widget_redraw(
    struct sw_switcher_widget *switcher_widget
) {
    gtk_widget_queue_draw(switcher_widget->window);
}

void sw_switcher_widget_mark_toplevel_selected(
    struct sw_switcher_widget *switcher_widget,
    struct sw_toplevel_widget *toplevel_widget
) {
    switcher_widget->selected_toplevel_widget = toplevel_widget;

    struct sw_polar_layout *primary_layout = switcher_widget->primary_layout;
    struct sw_flow_layout *slot_layout = switcher_widget->slot_layout;

    struct sw_border_item *border_item = switcher_widget->selection_border;

    struct sw_vec2 pos;
    struct sw_vec2 size;
    double corner_radius;
    double thickness = 3.0;
    double padding = 12.0;

    // Reset selected toplevel (called before navigation panel closed)
    if (toplevel_widget == NULL) {
        struct sw_polar_layout_item *item = primary_layout->center;
        
        pos = sw_vec2_add(item->pos, switcher_widget->primary_layout_pos);
        size = item->size;
        corner_radius = item->corner_radius;
    } else {
        struct sw_toplevel *selected_toplevel = toplevel_widget->toplevel;
        struct sw_polar_layout_item *primary_item = sw_polar_layout_find_by_data(primary_layout, selected_toplevel);
        
        if (primary_item != NULL) {
            pos = sw_vec2_add(primary_item->pos, switcher_widget->primary_layout_pos);
            size = primary_item->size;
            corner_radius = primary_item->corner_radius;
        }

        struct sw_flow_item *slot_item = sw_flow_layout_find_by_data(slot_layout, selected_toplevel);

        if (slot_item != NULL) {
            pos = sw_vec2_add(slot_item->pos, switcher_widget->slot_layout_pos);
            size = slot_item->size;
            corner_radius = slot_item->corner_radius;
        }
    }

    pos = sw_vec2_translate(pos, -padding / 2);
    size = sw_vec2_translate(size, padding);

    sw_border_item_set_thickness(border_item, thickness);
    sw_border_item_set_corner_radius(border_item, corner_radius);
    sw_border_item_set_size(border_item, size);

    struct sw_animation *pos_animation = sw_animation_create_squash_move(
        &border_item->pos,
        &border_item->size,
        size,
        pos, 
        0.35, 
        150, 
        sw_ease_out_cubic
    );

    sw_animation_manager_add(switcher_widget->animation_manager, pos_animation);
}

void sw_switcher_widget_notify_is_current_toplevel_change(
    struct sw_switcher_widget *switcher_widget,
    struct sw_toplevel_widget *toplevel_widget
) {
    switcher_widget->selected_toplevel_widget = toplevel_widget;

    struct sw_polar_layout *primary_layout = switcher_widget->primary_layout;
    struct sw_flow_layout *slot_layout = switcher_widget->slot_layout;

    struct sw_border_item *current_toplevel_item = switcher_widget->current_toplevel_border;

    struct sw_vec2 pos;
    struct sw_vec2 size;
    double corner_radius;
    double thickness = 3.0;
    double padding = 12.0;

    struct sw_toplevel *current_toplevel = toplevel_widget->toplevel;
    struct sw_polar_layout_item *primary_item = sw_polar_layout_find_by_data(primary_layout, current_toplevel);
        
    if (primary_item != NULL) {
        pos = sw_vec2_add(primary_item->pos, switcher_widget->primary_layout_pos);
        size = primary_item->size;
        corner_radius = primary_item->corner_radius;
    }

    struct sw_flow_item *slot_item = sw_flow_layout_find_by_data(slot_layout, current_toplevel);

    if (slot_item != NULL) {
        pos = sw_vec2_add(slot_item->pos, switcher_widget->slot_layout_pos);
        size = slot_item->size;
        corner_radius = slot_item->corner_radius;
    }

    pos = sw_vec2_translate(pos, -padding / 2);
    size = sw_vec2_translate(size, padding);

    sw_border_item_update(current_toplevel_item, size, corner_radius, thickness);
    sw_border_item_set_position(current_toplevel_item, pos);

    sw_switcher_widget_redraw(switcher_widget);
}

void sw_switcher_widget_enter_primary_zone(struct sw_switcher_widget *switcher_widget) {
    switcher_widget->current_zone = SW_SWITCHER_PRIMARY_ZONE;
}

void sw_switcher_widget_enter_slot_zone(struct sw_switcher_widget *switcher_widget) {
    switcher_widget->current_zone = SW_SWITCHER_SLOT_ZONE;
}

void sw_switcher_widget_destroy(struct sw_switcher_widget *switcher_widget) {
    free(switcher_widget);
}