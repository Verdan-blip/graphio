#include "cairo.h"
#include "glib.h"
#include <stdlib.h>
#include <wayland-util.h>

#include "include/ui/sw_switcher_widget.h"
#include "gtk/gtk.h"
#include "include/sw_graph_model.h"
#include "include/ui/sw_toplevel_widget.h"
#include "include/sw_switcher.h"
#include "include/sw_toplevel.h"

#define SLOT_H_RATIO 0.25
#define GAP_H_RATIO 0.05
#define PRIMARY_H_RATIO 0.70

#define RADIUS_RATIO 0.07
#define INNER_PAD_RATIO 0.04

static void draw_switcher_primary_background(
    cairo_t *cr,
    struct sw_switcher_widget *sw
) {
    double width = sw->primary_square_width;
    double height = sw->primary_square_width;

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

static void draw_switcher_slot_background(
    cairo_t *cr,
    struct sw_switcher_widget *sw
) {
    double width = sw->slot_rect_width;
    double height = sw->slot_rect_height;

    double x = sw->primary_square_horizontal_padding;
    double y = sw->slot_rect_height + sw->padding_between_containers;

    double radius = sw->corner_radius;
    double aspect = 1.0; 
    double degrees = G_PI / 180.0;

    cairo_new_sub_path(cr);
    cairo_arc(cr, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
    cairo_arc(cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
    cairo_arc(cr, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
    cairo_arc(cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
    cairo_close_path(cr);

    cairo_set_source_rgba(cr, 
        26 / 255.0f, 
        26 / 255.0f, 
        26 / 255.0f, 
        1.0
    );

    cairo_fill(cr);
}

static void draw_rounded_rect_panel(
    cairo_t *cr,
    double x,
    double y,
    double width,
    double height,
    double radius
) {
    double degrees = G_PI / 180.0;

    cairo_new_path(cr);

    cairo_arc(cr, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
    cairo_arc(cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
    cairo_arc(cr, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
    cairo_arc(cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
    
    cairo_close_path(cr);

    cairo_set_source_rgba(cr, 26 / 255.0f, 26 / 255.0f, 26 / 255.0f, 1.0);
    cairo_fill(cr);
}

static void update_toplevel_widgets_positions(
    struct sw_switcher_widget *switcher_widget
) {
    int cx = switcher_widget->primary_square_width / 2 + 
                switcher_widget->primary_square_horizontal_padding;

    int cy = switcher_widget->primary_square_height / 2 + 
                switcher_widget->slot_rect_height + 
                switcher_widget->padding_between_containers;

    struct sw_graph_model *graph = switcher_widget->model->graph_model;

    struct sw_graph_node *north = graph->north_node;
    struct sw_graph_node *south = graph->south_node;
    struct sw_graph_node *west = graph->west_node;
    struct sw_graph_node *east = graph->east_node;

    if (west != NULL) {
        struct sw_toplevel *toplevel = west->data;
        struct sw_toplevel_widget *w = toplevel->toplevel_widget;

        w->x = switcher_widget->inner_paddings + switcher_widget->primary_square_horizontal_padding;
        w->y = cy - w->height / 2;
    }

    if (east != NULL) {
        struct sw_toplevel *toplevel = east->data;
        struct sw_toplevel_widget *w = toplevel->toplevel_widget;

        w->x = switcher_widget->width - 
                    switcher_widget->primary_square_horizontal_padding - 
                    switcher_widget->inner_paddings - w->width;
        w->y = cy - w->height / 2;
    }

    if (north != NULL) {
        struct sw_toplevel *toplevel = north->data;
        struct sw_toplevel_widget *w = toplevel->toplevel_widget;

        w->x = cx - w->width / 2;
        w->y = switcher_widget->inner_paddings + 
                    switcher_widget->slot_rect_height + 
                    switcher_widget->padding_between_containers;
    }

    if (south != NULL) {
        struct sw_toplevel *toplevel = south->data;
        struct sw_toplevel_widget *w = toplevel->toplevel_widget;

        w->x = cx - w->width / 2;
        w->y = switcher_widget->height - 
                    switcher_widget->inner_paddings - 
                    w->height;
    }

    struct sw_graph_node *node;
    int i = 0;
    sw_graph_model_for_each_slot(node, graph) {
        struct sw_toplevel *slot_toplevel = node->data;

        struct sw_toplevel_widget *w = slot_toplevel->toplevel_widget;
        w->x = 24 + i * (w->width + 32);
        w->y = 24;
        i++;
    }
}

static void update_toplevel_widgets_sizes(
    struct sw_switcher_widget *switcher_widget
) {
    struct sw_graph_model *graph = switcher_widget->model->graph_model;
    struct sw_graph_node *node;
    sw_graph_model_for_each(node, graph) {
        struct sw_toplevel *toplevel = node->data;
        struct sw_toplevel_widget *toplevel_widget = toplevel->toplevel_widget;

        if (sw_graph_model_node_is_primary(graph, node)) {
            sw_toplevel_widget_primary_update_size(
                toplevel_widget, 
                switcher_widget->toplevel_primary_area_width, 
                switcher_widget->toplevel_primary_area_height
            );
        } else {
            sw_toplevel_widget_slot_update_size(
            toplevel_widget, 
            switcher_widget->toplevel_slot_area_width, 
            switcher_widget->toplevel_slot_area_height
            );
        }
    }
}

void sw_switcher_widget_init(struct sw_switcher *switcher, GtkWidget *window) {
    struct sw_switcher_widget *switcher_widget = calloc(1, sizeof(struct sw_switcher_widget));

    switcher_widget->model = switcher;
    switcher_widget->window = window;
    switcher_widget->width = 1;
    switcher_widget->height = 1;

    switcher_widget->is_slot_widget_visible = false;

    switcher_widget->current_zone = SW_SWITCHER_PRIMARY_ZONE;

    switcher_widget->selected_toplevel_widget = NULL;

    switcher->switcher_widget = switcher_widget;
}

void sw_switcher_widget_update_size(
    struct sw_switcher_widget *switcher_widget, 
    int width, int height
) {
    switcher_widget->width = width;
    switcher_widget->height = height;

    switcher_widget->slot_rect_height = height * SLOT_H_RATIO;
    switcher_widget->slot_rect_width = width; 

    switcher_widget->padding_between_containers = height * GAP_H_RATIO;

    switcher_widget->primary_square_height = height * PRIMARY_H_RATIO;
    
    double ideal_primary_w = switcher_widget->primary_square_height; 
    switcher_widget->primary_square_width = (ideal_primary_w > width) ? width : ideal_primary_w;

    switcher_widget->corner_radius = width * RADIUS_RATIO;
    switcher_widget->inner_paddings = width * INNER_PAD_RATIO;
    switcher_widget->paddings_between_toplevels = switcher_widget->inner_paddings;
    
    switcher_widget->primary_square_horizontal_padding = (width - switcher_widget->primary_square_width) / 2.0;

    // Content
    switcher_widget->content_width = switcher_widget->primary_square_width 
            - switcher_widget->inner_paddings * 2;

    switcher_widget->content_height = switcher_widget->primary_square_height
            - switcher_widget->inner_paddings * 2;

    int available_primary_toplevels_area_x = switcher_widget->content_width
            - switcher_widget->paddings_between_toplevels * 2;

    int available_primary_toplevels_area_y = switcher_widget->content_height
            - switcher_widget->paddings_between_toplevels * 2;

    switcher_widget->toplevel_primary_area_width = available_primary_toplevels_area_x / 3;
    switcher_widget->toplevel_primary_area_height = available_primary_toplevels_area_y / 3;

    switcher_widget->toplevel_slot_area_width = switcher_widget->slot_rect_height - 2 * 24;
    switcher_widget->toplevel_slot_area_height = switcher_widget->toplevel_slot_area_width;

    update_toplevel_widgets_sizes(switcher_widget);
    update_toplevel_widgets_positions(switcher_widget);
}

void sw_switcher_widget_on_add_toplevel(
    struct sw_switcher_widget *switcher_widget,
    struct sw_toplevel_widget *toplevel
) {
    struct sw_graph_model *graph = switcher_widget->model->graph_model;

    if (sw_graph_model_slot_nodes_empty(graph)) {
        switcher_widget->is_slot_widget_visible = false;
    } else {
        switcher_widget->is_slot_widget_visible = true;
    }

    update_toplevel_widgets_sizes(switcher_widget);
    update_toplevel_widgets_positions(switcher_widget);

    if (switcher_widget->window) sw_switcher_widget_redraw(switcher_widget);
}

void sw_switcher_widget_on_remove_toplevel(
    struct sw_switcher_widget *switcher_widget,
    struct sw_toplevel_widget *toplevel_widget
) {
    struct sw_graph_model *graph = switcher_widget->model->graph_model;

    if (sw_graph_model_slot_nodes_empty(graph)) {
        switcher_widget->is_slot_widget_visible = false;
    } else {
        switcher_widget->is_slot_widget_visible = true;
    }

    update_toplevel_widgets_sizes(switcher_widget);
    update_toplevel_widgets_positions(switcher_widget);

    if (switcher_widget->selected_toplevel_widget == toplevel_widget) {
        switcher_widget->selected_toplevel_widget = NULL;
    }

    if (switcher_widget->window) sw_switcher_widget_redraw(switcher_widget);
}

void sw_switcher_widget_on_update_toplevel(struct sw_switcher_widget *switcher_widget) {
    update_toplevel_widgets_sizes(switcher_widget);
    update_toplevel_widgets_positions(switcher_widget);

    if (switcher_widget->window) sw_switcher_widget_redraw(switcher_widget);
}

static void sw_switcher_draw_placeholder_by_index(
    struct sw_switcher_widget *switcher_widget,
    int index,
    cairo_t *cr
) {
    int x = 0, y = 0;

    switch (index) {
        case INDEX_TOP: {
            x = switcher_widget->primary_square_horizontal_padding + 
                    switcher_widget->inner_paddings +
                    switcher_widget->toplevel_primary_area_width +
                    switcher_widget->paddings_between_toplevels;

            y = switcher_widget->slot_rect_height +
                    switcher_widget->padding_between_containers +
                    switcher_widget->inner_paddings;
            break;
        }

        case INDEX_BOTTOM: {
            x = switcher_widget->primary_square_horizontal_padding + 
                    switcher_widget->inner_paddings +
                    switcher_widget->toplevel_primary_area_width +
                    switcher_widget->paddings_between_toplevels;

            y = switcher_widget->height -
                    switcher_widget->toplevel_primary_area_height -
                    switcher_widget->inner_paddings;
            break;
        }

         case INDEX_LEFT: {
            x = switcher_widget->primary_square_horizontal_padding +
                    switcher_widget->inner_paddings;

            y = switcher_widget->height -
                    switcher_widget->toplevel_primary_area_height -
                    switcher_widget->inner_paddings -
                    switcher_widget->toplevel_primary_area_height -
                    switcher_widget->paddings_between_toplevels;
            break;
         }

         case INDEX_RIGHT: {
            x = switcher_widget->width -
                    switcher_widget->primary_square_horizontal_padding -
                    switcher_widget->inner_paddings -
                    switcher_widget->toplevel_primary_area_width;

            y = switcher_widget->height -
                    switcher_widget->toplevel_primary_area_height -
                    switcher_widget->inner_paddings -
                    switcher_widget->toplevel_primary_area_height -
                    switcher_widget->paddings_between_toplevels;
            break;
         }
    }

    sw_toplevel_widget_draw_placeholder(
        x, 
        y, 
        switcher_widget->toplevel_primary_area_width, 
        switcher_widget->toplevel_primary_area_height, 
        switcher_widget->corner_radius, 
        cr
    );
}

void sw_switcher_widget_draw(
    struct sw_switcher_widget *switcher_widget,
    cairo_t *cr
) {
    if (switcher_widget->is_slot_widget_visible) {
        draw_rounded_rect_panel(
            cr, 
            0, 
            0,
            switcher_widget->slot_rect_width,
            switcher_widget->slot_rect_height,
            switcher_widget->corner_radius
        );
    }

    draw_rounded_rect_panel(
        cr, 
        switcher_widget->primary_square_horizontal_padding, 
        switcher_widget->slot_rect_height + switcher_widget->padding_between_containers,
        switcher_widget->primary_square_width,
        switcher_widget->primary_square_height,
        switcher_widget->corner_radius
    );

    struct sw_graph_model *graph = switcher_widget->model->graph_model;
    struct sw_graph_node *node;
    int index = 0;
    sw_graph_model_for_each_primary_unsafe(node, index, graph) {
        if (node == NULL) {
            sw_switcher_draw_placeholder_by_index(switcher_widget, index, cr);
            continue;
        }
        
        struct sw_toplevel* toplevel = node->data;
        struct sw_toplevel_widget *toplevel_widget = toplevel->toplevel_widget;
        sw_toplevel_widget_draw(toplevel_widget, switcher_widget, cr);
    }

    if (switcher_widget->selected_toplevel_widget == NULL) {
        int x = switcher_widget->primary_square_horizontal_padding + 
                    switcher_widget->inner_paddings +
                    switcher_widget->toplevel_primary_area_width +
                    switcher_widget->paddings_between_toplevels;


        int y = switcher_widget->slot_rect_height +
                    switcher_widget->padding_between_containers +
                    switcher_widget->inner_paddings +
                    switcher_widget->toplevel_primary_area_height +
                    switcher_widget->paddings_between_toplevels;
        
        sw_toplevel_widget_draw_selection(
            x, 
            y, 
            switcher_widget->toplevel_primary_area_width, 
            switcher_widget->toplevel_primary_area_height, 
            switcher_widget->corner_radius, 
            cr
        );
    }

    struct sw_graph_node *slot_node;
    sw_graph_model_for_each_slot(slot_node, graph) {
        struct sw_toplevel *toplevel = slot_node->data;
        sw_toplevel_widget_draw(toplevel->toplevel_widget, switcher_widget, cr);
    }
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