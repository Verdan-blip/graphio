#define _POSIX_C_SOURCE 200809L

#include <gtk/gtk.h>
#include <gtk-layer-shell.h>

static gboolean draw_cb(GtkWidget *widget, cairo_t *cr, gpointer data) {
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);

    cairo_set_source_rgb(cr, 0.15, 0.15, 0.15);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, height * 0.5);
    cairo_move_to(cr, 10, height * 0.7);
    cairo_show_text(cr, "ALTAB");
    return TRUE;
}

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    GdkDisplay *display = gdk_display_get_default();
    int n_monitors = gdk_display_get_n_monitors(display);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_widget_realize(window);

    gtk_layer_init_for_window(GTK_WINDOW(window));
    gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_OVERLAY);

    gtk_widget_set_size_request(window, 400, 100);

    gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, -1);
    gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_BOTTOM, -1);
    gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_LEFT, -1);
    gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, -1);

    GtkWidget *area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), area);

    g_signal_connect(area, "draw", G_CALLBACK(draw_cb), NULL);

    gtk_widget_show_all(window);

    gtk_main();
    return 0;
}