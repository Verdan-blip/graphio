#include <gtk/gtk.h>
#include <gtk-layer-shell.h>
#include <gdk/gdkwayland.h>

#include "include/app/sw_application.h"
#include "include/input/sw_input_handler.h"

static const int WINDOW_WIDTH = 468;
static const int WINDOW_HEIGHT = 468;

static gboolean app_on_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    struct sw_application *app = data;
    sw_switcher_widget_draw(app->switcher->switcher_widget, cr);
    return TRUE;
}

static gboolean app_on_tick(GtkWidget *widget, GdkFrameClock *frame_clock, gpointer data) {
    struct sw_application *app = data;
    struct sw_animation_manager *am = app->switcher->switcher_widget->animation_manager;
    
    sw_animation_manager_update(am, gdk_frame_clock_get_frame_time(frame_clock));
    gtk_widget_queue_draw(widget);
    return G_SOURCE_CONTINUE;
}

static gboolean app_on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    struct sw_application *app = data;
    return sw_input_handler_handle_key_press(app->switcher->switcher_widget, event);
}

static gboolean app_on_key_release(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    struct sw_application *app = data;
    return sw_input_handler_handle_release(app->switcher->switcher_widget, event);
}

static void app_on_size_allocate(GtkWidget *widget, GdkRectangle *allocation, gpointer data) {
    struct sw_application *app = data;
    sw_switcher_widget_update_size(
        app->switcher->switcher_widget, 
        sw_vec2_create(allocation->width, allocation->height)
    );
}

struct sw_application* sw_application_create(int *argc, char ***argv) {
    gtk_init(argc, argv);

    struct sw_application *app = calloc(1, sizeof(struct sw_application));
    if (!app) return NULL;

    app->switcher = sw_switcher_create();

    GdkDisplay *gdk_display = gdk_display_get_default();
    struct wl_display *wl_display = gdk_wayland_display_get_wl_display(gdk_display);
    app->backend = sw_wayland_backend_create(wl_display, app->switcher);
    sw_wayland_backend_sync(app->backend);

    app->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    app->draw_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(app->window), app->draw_area);

    sw_switcher_widget_init(app->switcher, app->draw_area);

    gtk_layer_init_for_window(GTK_WINDOW(app->window));
    gtk_layer_set_layer(GTK_WINDOW(app->window), GTK_LAYER_SHELL_LAYER_OVERLAY);
    gtk_layer_set_keyboard_interactivity(GTK_WINDOW(app->window), TRUE);
    
    gtk_widget_set_size_request(app->window, WINDOW_WIDTH, WINDOW_HEIGHT);

    gtk_layer_set_margin(GTK_WINDOW(app->window), GTK_LAYER_SHELL_EDGE_TOP, -1);
    gtk_layer_set_margin(GTK_WINDOW(app->window), GTK_LAYER_SHELL_EDGE_BOTTOM, -1);
    gtk_layer_set_margin(GTK_WINDOW(app->window), GTK_LAYER_SHELL_EDGE_LEFT, -1);
    gtk_layer_set_margin(GTK_WINDOW(app->window), GTK_LAYER_SHELL_EDGE_RIGHT, -1);

    GdkVisual *visual = gdk_screen_get_rgba_visual(gtk_widget_get_screen(app->window));
    if (visual) gtk_widget_set_visual(app->window, visual);
    gtk_widget_set_app_paintable(app->window, TRUE);

    g_signal_connect(app->window, "key-press-event", G_CALLBACK(app_on_key_press), app);
    g_signal_connect(app->window, "key-release-event", G_CALLBACK(app_on_key_release), app);
    g_signal_connect(app->draw_area, "draw", G_CALLBACK(app_on_draw), app);
    g_signal_connect(app->draw_area, "size-allocate", G_CALLBACK(app_on_size_allocate), app);

    gtk_widget_add_tick_callback(app->window, (GtkTickCallback)app_on_tick, app, NULL);

    gtk_widget_show_all(app->window);
    gtk_widget_grab_focus(app->window);

    return app;
}

int sw_application_run(struct sw_application *app) {
    if (!app) return 1;
    gtk_main();
    return 0;
}

void sw_application_destroy(struct sw_application *app) {
    if (!app) return;

    sw_wayland_backend_destroy(app->backend);
    sw_switcher_widget_destroy(app->switcher->switcher_widget);
    sw_switcher_destroy(app->switcher);
    
    free(app);
}