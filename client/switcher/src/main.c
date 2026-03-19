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

#include "include/sw_toplevel.h"
#include "include/sw_switcher.h"
#include "include/ui/sw_switcher_widget.h"

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

gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    struct sw_switcher_widget *switcher_widget = data;
    struct sw_switcher *switcher = switcher_widget->model;

    switch (event->keyval) {
        case GDK_KEY_Up: {
            sw_switcher_set_primary_activated(switcher, INDEX_TOP);
            return true;
        }

        case GDK_KEY_Down: {
            sw_switcher_set_primary_activated(switcher, INDEX_BOTTOM);
            return true;
        }

        case GDK_KEY_Right: {
            sw_switcher_set_primary_activated(switcher, INDEX_RIGHT);
            return true;
        }

        case GDK_KEY_Left: {
            sw_switcher_set_primary_activated(switcher, INDEX_LEFT);
            return true;
        }
    }

    return false;
}

gboolean on_key_release(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    struct sw_switcher_widget *switcher_widget = data;

    if (event->keyval == GDK_KEY_C) {
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

    int width = 324;
    int height = 324;

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