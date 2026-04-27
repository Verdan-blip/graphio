#include "cairo.h"
#include <stdlib.h>
#include <wayland-util.h>

#include "include/ui/sw_switcher_widget.h"
#include "include/math/sw_vec2.h"
#include "include/ui/animation/sw_animation_manager.h"
#include "include/ui/sw_border_item.h"
#include "include/ui/sw_flow_layout.h"
#include "include/ui/sw_polar_layout.h"
#include "gtk/gtk.h"
#include "include/sw_graph_model.h"
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
    switcher_widget->width = 1;
    switcher_widget->height = 1;

    switcher_widget->primary_layout = sw_polar_layout_create();
    switcher_widget->slot_layout = sw_flow_layout_create();

    switcher_widget->selection_border = sw_border_item_create();
    switcher_widget->current_toplevel_border = sw_border_item_create();

    switcher_widget->is_slot_widget_visible = false;

    switcher_widget->current_zone = SW_SWITCHER_PRIMARY_ZONE;

    switcher_widget->selected_toplevel_widget = NULL;

    switcher_widget->primary_bg_active_color = malloc(4 * sizeof(float));
    switcher_widget->primary_bg_active_color[0] = 0.10f;
    switcher_widget->primary_bg_active_color[1] = 0.11f;
    switcher_widget->primary_bg_active_color[2] = 0.15f;
    switcher_widget->primary_bg_active_color[3] = 0.85f;

    switcher_widget->primary_bg_inactive_color = malloc(4 * sizeof(float));
    switcher_widget->primary_bg_inactive_color[0] = 0.10f;
    switcher_widget->primary_bg_inactive_color[1] = 0.11f;
    switcher_widget->primary_bg_inactive_color[2] = 0.15f;
    switcher_widget->primary_bg_inactive_color[3] = 0.55f;

    switcher_widget->slot_bg_active_color = malloc(4 * sizeof(float));
    switcher_widget->slot_bg_active_color[0] = 0.10f;
    switcher_widget->slot_bg_active_color[1] = 0.11f;
    switcher_widget->slot_bg_active_color[2] = 0.15f;
    switcher_widget->slot_bg_active_color[3] = 0.85f;

    switcher_widget->slot_bg_inactive_color = malloc(4 * sizeof(float));
    switcher_widget->slot_bg_inactive_color[0] = 0.10f;
    switcher_widget->slot_bg_inactive_color[1] = 0.11f;
    switcher_widget->slot_bg_inactive_color[2] = 0.15f;
    switcher_widget->slot_bg_inactive_color[3] = 0.55f;

    switcher_widget->animation_manager = sw_animation_manager_create();

    switcher->switcher_widget = switcher_widget;
}

void sw_switcher_widget_update_size(
    struct sw_switcher_widget *switcher_widget, 
    int width, int height
) {
    switcher_widget->width = width;
    switcher_widget->height = height;

    double slot_height = height * SLOT_H_RATIO;
    struct sw_vec2 primary_size = sw_vec2_create(height * PRIMARY_H_RATIO, height * PRIMARY_H_RATIO);
    double gap = height - slot_height - primary_size.y;

    double slot_inner_padding = 24;
    double slot_gap_between_items = 32;
    double slot_item_height = slot_height - slot_inner_padding * 2;
    sw_flow_layout_resize(
        switcher_widget->slot_layout,
        width,
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

    switcher_widget->primary_layout_pos.x = (width - switcher_widget->primary_layout->canvas_size.x) / 2;
    switcher_widget->primary_layout_pos.y = slot_height + gap;

    switcher_widget->slot_layout_pos.x = 0;
    switcher_widget->slot_layout_pos.y = 0;

    switcher_widget->padding_between_layouts = gap;
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

static void sw_switcher_draw_primary_item(
    struct sw_polar_layout *primary_layout,
    struct sw_polar_layout_item *item,
    cairo_t *cr
) {
    sw_toplevel_widget_draw_placeholder(
        item->pos, 
        primary_layout->canvas_size, 
        primary_layout->corner_radius, 
        cr
    );
}

void sw_switcher_widget_draw(
    struct sw_switcher_widget *switcher_widget,
    cairo_t *cr
) {
    // Drawing slot toplevels background
    struct sw_flow_layout *slot_layout = switcher_widget->slot_layout;
    if (switcher_widget->is_slot_widget_visible) {
        float *slot_bg_color;
        if (switcher_widget->current_zone == SW_SWITCHER_SLOT_ZONE) {
            slot_bg_color = switcher_widget->slot_bg_active_color;
        } else {
            slot_bg_color = switcher_widget->slot_bg_inactive_color;
        }

        sw_draw_filled_round_corner_rect(
            switcher_widget->slot_layout_pos,
            sw_vec2_create(
                switcher_widget->slot_layout->max_width, 
                switcher_widget->slot_layout->total_height
            ),
            switcher_widget->slot_layout->corner_radius, 
            slot_bg_color, 
            cr
        );
    }

    // Drawing primary toplevels background
    struct sw_polar_layout *primary_layout = switcher_widget->primary_layout;

    float *primary_bg_color;
    if (switcher_widget->current_zone != SW_SWITCHER_PRIMARY_ZONE) {
        primary_bg_color = switcher_widget->slot_bg_inactive_color;
    } else {
        primary_bg_color = switcher_widget->slot_bg_active_color;
    }

    sw_draw_filled_round_corner_rect(
        switcher_widget->primary_layout_pos, 
        switcher_widget->primary_layout->canvas_size,
        switcher_widget->primary_layout->corner_radius, 
        primary_bg_color, 
        cr
    );

    // Drawing primary toplevels
    struct sw_polar_layout_item *primary_item;
    for (int i = 0; i < 4; i++) {
        switch (i) {
            case 0: primary_item = primary_layout->west; break;
            case 1: primary_item = primary_layout->east; break;
            case 2: primary_item = primary_layout->north; break;
            default: primary_item = primary_layout->south; break;
        }

        struct sw_vec2 global_item_pos = sw_vec2_add(primary_item->pos, switcher_widget->primary_layout_pos);

        if (primary_item->data == NULL) {
            sw_toplevel_widget_draw_placeholder(
                global_item_pos, 
                primary_item->size,
                primary_item->corner_radius, 
                cr
            );
        } else {
            struct sw_toplevel *toplevel = primary_item->data;
            sw_toplevel_widget_draw(
                toplevel->toplevel_widget,
                switcher_widget,
                global_item_pos,
                primary_item->size, 
                cr
            );
        }
    }

    // Drawing slot toplevels
    struct sw_flow_item *slot_item;
    for (int i = 0; i < slot_layout->items_count; i++) {
        slot_item = slot_layout->items[i];

        struct sw_toplevel *item_toplevel = slot_item->data;
        struct sw_toplevel_widget *item_toplevel_widget = item_toplevel->toplevel_widget;

        sw_toplevel_widget_draw(
            item_toplevel_widget,
            switcher_widget,
            sw_vec2_add(slot_item->pos, switcher_widget->slot_layout_pos),
            slot_item->size,
            cr
        );
    }

    // Drawing current toplevel border
    struct sw_border_item *current_toplevel_border = switcher_widget->current_toplevel_border;
    float current_toplevel_border_color[4] = {
        0.68f, 0.68f, 1.0f, 0.5f
    };

    sw_draw_outlined_round_corner_rect(
        current_toplevel_border->pos, 
        current_toplevel_border->size,
        current_toplevel_border->corner_radius, 
        current_toplevel_border->thickness,
        current_toplevel_border_color,
        cr
    );

    // Drawing selection border
    struct sw_border_item *selection_border = switcher_widget->selection_border;
    float selection_border_color[4] = {
        0.68f, 0.68f, 1.0f, 1.0f
    };

    sw_draw_outlined_round_corner_rect(
        selection_border->pos, 
        selection_border->size,
        selection_border->corner_radius, 
        selection_border->thickness,
        selection_border_color,
        cr
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

    sw_border_item_resize(border_item, size, corner_radius, thickness);
    sw_border_item_set_position(border_item, pos);

    sw_switcher_widget_redraw(switcher_widget);
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

    sw_border_item_resize(current_toplevel_item, size, corner_radius, thickness);
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