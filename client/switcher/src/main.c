#include "include/sw_graph_model.h"
#define _POSIX_C_SOURCE 200809L

#include <wayland-client-protocol.h>
#include <wayland-server-protocol.h>
#include <wayland-client-core.h>
#include <wayland-util.h>

#include "glib.h"
#include <gtk/gtk.h>
#include <gtk-layer-shell.h>
#include <gdk/gdkwayland.h>
#include "gdk/gdkkeysyms.h"
#include "gdk/gdk.h"
#include "glibconfig.h"

#include "include/sw_toplevel.h"
#include "include/sw_switcher.h"
#include "include/ui/sw_switcher_widget.h"
#include "include/ui/sw_toplevel_widget.h"

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    struct sw_switcher_widget *switcher_widget = data;

    sw_switcher_widget_draw(switcher_widget, cr);

    return TRUE;
}

static gboolean on_area_size_allocate(GtkWidget *widget, GdkRectangle *allocation, gpointer data) {
    struct sw_switcher_widget *switcher_widget = data;

    sw_switcher_widget_update_size(switcher_widget, allocation->width, allocation->height);

    return TRUE;
}

static gboolean handle_primary_toplevel_activation(
    struct sw_switcher_widget *switcher_widget,
    struct sw_toplevel *primary_toplevel
) {
    struct sw_switcher *switcher = switcher_widget->model;

    if (primary_toplevel == NULL) return false;

    sw_switcher_widget_mark_toplevel_selected(
        switcher_widget, 
        primary_toplevel->toplevel_widget
    );

    return true;
}

static gboolean on_key_press_in_primary_zone(
    struct sw_switcher_widget *switcher_widget,
    guint keyval
) {
    struct sw_switcher *switcher = switcher_widget->model;
    struct sw_graph_model *graph = switcher->graph_model;

    switch (keyval) {
        case GDK_KEY_Up: {
            struct sw_graph_node *node = graph->north_node;

            if (node == NULL) return false;

            struct sw_toplevel *north_toplevel = node->data;
            if (north_toplevel == NULL) return false;

            struct sw_toplevel_widget *selected_toplevel_widget = switcher_widget->selected_toplevel_widget;

            if (selected_toplevel_widget == NULL) {
                return handle_primary_toplevel_activation(
                    switcher_widget, 
                    north_toplevel
                );
            }

            struct sw_toplevel *selected = switcher_widget->selected_toplevel_widget->model;

            if (selected == north_toplevel) {
                if (sw_graph_model_slot_nodes_empty(graph)) return false;

                struct sw_graph_node *first_slot_node = sw_graph_model_get_first_slot(graph);
                if (first_slot_node == NULL) return false;

                struct sw_toplevel *first_slot_toplevel = first_slot_node->data;

                if (first_slot_toplevel == NULL) return false;

                sw_switcher_widget_mark_toplevel_selected(
                    switcher_widget, 
                    first_slot_toplevel->toplevel_widget
                );

                sw_switcher_widget_enter_slot_zone(switcher_widget);

                return false;
            }

            return handle_primary_toplevel_activation(
                switcher_widget, 
                north_toplevel
            );
        }

        case GDK_KEY_Down: {
            struct sw_graph_node *south_node = graph->south_node;
            if (south_node == NULL) return false;

            return handle_primary_toplevel_activation(
                switcher_widget, 
                south_node->data
            );
        }

        case GDK_KEY_Right: {
            struct sw_graph_node *east_node = graph->east_node;
            if (east_node == NULL) return false;

            return handle_primary_toplevel_activation(
                switcher_widget, 
                east_node->data
            );
        }

        case GDK_KEY_Left: {
            struct sw_graph_node *west_node = graph->west_node;
            if (west_node == NULL) return false;

            return handle_primary_toplevel_activation(
                switcher_widget, 
                west_node->data
            );
        }
    }

    return false;
}

static gboolean on_key_press_in_slot_zone(
    struct sw_switcher_widget *switcher_widget,
    guint keyval
) {
    struct sw_switcher *switcher = switcher_widget->model;
    struct sw_graph_model *graph = switcher->graph_model;

    struct sw_toplevel *current = switcher_widget->selected_toplevel_widget->model;

    switch (keyval) {
        case GDK_KEY_Down: {
            struct sw_toplevel *north_toplevel = graph->north_node->data;
            sw_switcher_widget_enter_primary_zone(switcher_widget);
            handle_primary_toplevel_activation(switcher_widget, north_toplevel);
            return true;
        }

        case GDK_KEY_Right: {
            struct sw_graph_node *current_node = current->node;
            struct sw_graph_node *next_node = current_node->next;

            if (next_node == NULL) return false;

            struct sw_toplevel *next_toplevel = next_node->data;

            sw_switcher_widget_mark_toplevel_selected(
                switcher_widget, 
                next_toplevel->toplevel_widget
            );
            return true;
        }

        case GDK_KEY_Left: {
            struct sw_graph_node *current_node = current->node;
            struct sw_graph_node *prev_node = current_node->prev;

            if (prev_node == NULL) return false;

            struct sw_toplevel *prev_toplevel = prev_node->data;

            sw_switcher_widget_mark_toplevel_selected(
                switcher_widget, 
                prev_toplevel->toplevel_widget
            );
            return true;
        }
    }

    return false;
}

gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    struct sw_switcher_widget *switcher_widget = data;
    struct sw_switcher *switcher = switcher_widget->model;

    if (switcher_widget->current_zone == SW_SWITCHER_PRIMARY_ZONE) {
        return on_key_press_in_primary_zone(switcher_widget, event->keyval);
    }

    if (switcher_widget->current_zone == SW_SWITCHER_SLOT_ZONE) {
        return on_key_press_in_slot_zone(switcher_widget, event->keyval);
    }

    return false;
}

gboolean on_key_release(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    struct sw_switcher_widget *switcher_widget = data;
    struct sw_switcher *switcher = switcher_widget->model;

    struct sw_toplevel_widget *selected = switcher_widget->selected_toplevel_widget;

    if (selected == NULL) return true;

    if (event->keyval == GDK_KEY_Super_L) {
        sw_switcher_set_activated(switcher, selected->model);

        sw_switcher_widget_enter_primary_zone(switcher_widget);
        sw_switcher_widget_mark_toplevel_selected(switcher_widget, NULL);
        return true;
    }

    return false;
}

static void registry_handle_global(
    void *data, 
    struct wl_registry *registry, 
    uint32_t name, 
    const char *interface, 
    uint32_t version
) {
    struct sw_switcher *switcher = data;

    if (strcmp(interface, zwlr_foreign_toplevel_manager_v1_interface.name) == 0) {
        struct zwlr_foreign_toplevel_manager_v1 *manager = wl_registry_bind(
            registry, 
            name, 
            &zwlr_foreign_toplevel_manager_v1_interface, 
            1
        );

        switcher->manager = manager;
        
        static const struct zwlr_foreign_toplevel_manager_v1_listener manager_impl = {
            .toplevel = sw_handle_toplevel
        };

        zwlr_foreign_toplevel_manager_v1_add_listener(manager, &manager_impl, switcher);
    }

    if (strcmp(interface, wl_seat_interface.name) == 0) {
        switcher->seat = wl_registry_bind(registry, name, &wl_seat_interface, version);
    }
}

static void registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name) {

}

static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    GdkDisplay *display = gdk_display_get_default();
    struct wl_display *wl_display = gdk_wayland_display_get_wl_display(display);

    // Switcher model
    struct sw_switcher *switcher = sw_switcher_create();

    struct wl_registry *registry = wl_display_get_registry(wl_display);
    wl_registry_add_listener(registry, &registry_listener, switcher);

    wl_display_roundtrip(wl_display);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    int width = 468;
    int height = 468;

    GtkWidget *area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), area);
    
    sw_switcher_widget_init(switcher, area);

    GdkVisual *visual = gdk_screen_get_rgba_visual(gtk_widget_get_screen(window));
    if (visual) gtk_widget_set_visual(window, visual);
    
    gtk_widget_set_app_paintable(window, TRUE);
    gtk_widget_realize(window);

    gtk_layer_init_for_window(GTK_WINDOW(window));
    gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_OVERLAY);
    gtk_layer_set_keyboard_interactivity(GTK_WINDOW(window), TRUE);

    gtk_widget_set_size_request(window, width, height);

    gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, -1);
    gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_BOTTOM, -1);
    gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_LEFT, -1);
    gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, -1);

    gtk_widget_add_events(window, GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);

    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), switcher->switcher_widget);
    g_signal_connect(window, "key-release-event", G_CALLBACK(on_key_release), switcher->switcher_widget);

    g_signal_connect(area, "draw", G_CALLBACK(on_draw), switcher->switcher_widget);
    g_signal_connect(area, "size-allocate", G_CALLBACK(on_area_size_allocate), switcher->switcher_widget);

    gtk_widget_show_all(window);

    gtk_widget_set_can_focus(window, TRUE);
    gtk_widget_grab_focus(window);

    gtk_main();

    sw_switcher_widget_destroy(switcher->switcher_widget);
    sw_switcher_destroy(switcher);
    return 0;
}