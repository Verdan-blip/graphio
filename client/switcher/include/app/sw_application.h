#ifndef SW_APPLICATION_H
#define SW_APPLICATION_H

#include <gtk/gtk.h>

#include "include/sw_switcher.h"
#include "include/wayland/sw_wayland_backend.h"

struct sw_application {
    GtkWidget *window;
    GtkWidget *draw_area;
    struct sw_switcher *switcher;
    struct sw_wayland_backend *backend;
};

struct sw_application* sw_application_create(int *argc, char ***argv);
int sw_application_run(struct sw_application *app);
void sw_application_destroy(struct sw_application *app);

#endif