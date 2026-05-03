#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk-layer-shell.h>
#include <cairo.h>
#include <math.h>

char* get_current_dir() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        return strdup(cwd);
    }

    return NULL;
}

static cairo_surface_t *image_surface;

static gboolean draw_cb(GtkWidget *widget, cairo_t *cr, gpointer data) {
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);

    printf("%d, %d", width, height);

    double img_w = cairo_image_surface_get_width(image_surface);
    double img_h = cairo_image_surface_get_height(image_surface);

    double scale = fmax((double)width / img_w, (double)height / img_h);
    double new_w = img_w * scale;
    double new_h = img_h * scale;

    double offset_x = (width - new_w) / 2.0;
    double offset_y = (height - new_h) / 2.0;

    cairo_save(cr);
    cairo_translate(cr, offset_x, offset_y);
    cairo_scale(cr, scale, scale);
    cairo_set_source_surface(cr, image_surface, 0, 0);
    cairo_paint(cr);
    cairo_restore(cr);

    return FALSE;
}

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    GdkDisplay *display = gdk_display_get_default();
    int n_monitors = gdk_display_get_n_monitors(display);

    for (int i = 0; i < n_monitors; i++) {
        GdkMonitor *monitor = gdk_display_get_monitor(display, i);
        GdkRectangle geometry;
        gdk_monitor_get_geometry(monitor, &geometry);

        GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_layer_init_for_window(GTK_WINDOW(window));
        gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_BACKGROUND);
        gtk_layer_set_monitor(GTK_WINDOW(window), monitor);

        gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
        gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
        gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
        gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);

        char *wallpaper_path = strcat(get_current_dir(), "/res/wallpaper.png");
        image_surface = cairo_image_surface_create_from_png(wallpaper_path);

        cairo_status_t status = cairo_surface_status(image_surface);
        if (status != CAIRO_STATUS_SUCCESS) {
            const char *message = cairo_status_to_string(status);
            printf("cairo: failed to load image: %s\n", message);
        }

        GtkWidget *area = gtk_drawing_area_new();
        gtk_container_add(GTK_CONTAINER(window), area);
        g_signal_connect(area, "draw", G_CALLBACK(draw_cb), NULL);

        gtk_widget_show_all(window);
    }

    gtk_main();

    return 0;
};