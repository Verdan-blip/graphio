#include <gdk/gdkkeysyms.h>

#include "include/input/sw_input_handler.h"
#include "include/sw_toplevel.h"
#include "include/ui/sw_toplevel_widget.h"
#include "include/sw_graph_model.h"

static gboolean handle_primary_activation(
    struct sw_switcher_widget *sw, 
    struct sw_graph_node *node
) {
    if (!node || !node->data) return FALSE;
    
    struct sw_toplevel *toplevel = node->data;
    sw_switcher_widget_mark_toplevel_selected(sw, toplevel->toplevel_widget);
    return TRUE;
}

static gboolean handle_first_slot_activation(
    struct sw_switcher_widget *sw, 
    struct sw_graph_node *slot_head
) {
    if (!slot_head || !slot_head->data) return FALSE;
    
    struct sw_toplevel *toplevel =slot_head->data;
    sw_switcher_widget_mark_toplevel_selected(sw, toplevel->toplevel_widget);
    sw_switcher_widget_enter_slot_zone(sw);

    return TRUE;
}

static gboolean handle_primary_zone(struct sw_switcher_widget *sw, guint keyval) {
    struct sw_graph_model *graph = sw->switcher->graph_model;

    switch (keyval) {
        case GDK_KEY_Up: {
            struct sw_toplevel *selected = sw->selected_toplevel_widget->toplevel;
            struct sw_graph_node *north = graph->north_node;

            if (north == NULL) return FALSE;

            if (north->data == selected) {
                return handle_first_slot_activation(sw, graph->slot_head);
            } else {
                return handle_primary_activation(sw, graph->north_node);
            }
        }
        
        case GDK_KEY_Down:
            return handle_primary_activation(sw, graph->south_node);
            
        case GDK_KEY_Right:
            return handle_primary_activation(sw, graph->east_node);
            
        case GDK_KEY_Left:
            return handle_primary_activation(sw, graph->west_node);
    }
    return FALSE;
}

static gboolean handle_slot_zone(struct sw_switcher_widget *sw, guint keyval) {
    if (!sw->selected_toplevel_widget) return FALSE;
    
    struct sw_graph_model *graph = sw->switcher->graph_model;
    struct sw_toplevel *current = sw->selected_toplevel_widget->toplevel;
    struct sw_graph_node *current_node = current->node;

    switch (keyval) {
        case GDK_KEY_Down:
            sw_switcher_widget_enter_primary_zone(sw);
            return handle_primary_activation(sw, graph->north_node);

        case GDK_KEY_Right:
            if (current_node->next) {
                struct sw_toplevel *next = current_node->next->data;
                sw_switcher_widget_mark_toplevel_selected(sw, next->toplevel_widget);
                return TRUE;
            }
            break;

        case GDK_KEY_Left:
            if (current_node->prev) {
                struct sw_toplevel *prev = current_node->prev->data;
                sw_switcher_widget_mark_toplevel_selected(sw, prev->toplevel_widget);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

static gboolean handle_primary_zone_bordering(struct sw_switcher_widget *sw) {
    struct sw_graph_model *graph = sw->switcher->graph_model;

    return handle_first_slot_activation(sw, graph->slot_head);
}

gboolean sw_input_handler_handle_key_press(
    struct sw_switcher_widget *sw, 
    GdkEventKey *event
) {
    if (!sw) return FALSE;

    if (sw->current_zone == SW_SWITCHER_PRIMARY_ZONE) {
        return handle_primary_zone(sw, event->keyval);
    } 
    
    if (sw->current_zone == SW_SWITCHER_SLOT_ZONE) {
        return handle_slot_zone(sw, event->keyval);
    }

    return FALSE;
}

gboolean sw_input_handler_handle_release(struct sw_switcher_widget *sw, GdkEventKey *event) {
    if (event->keyval == GDK_KEY_Super_L || event->keyval == GDK_KEY_Super_R) {
        if (sw->selected_toplevel_widget) {
            sw_switcher_set_activated(sw->switcher, sw->selected_toplevel_widget->toplevel);
            
            sw_switcher_widget_enter_primary_zone(sw);
            sw_switcher_widget_mark_toplevel_selected(sw, NULL);
            
            return TRUE;
        }
    }
    return FALSE;
}