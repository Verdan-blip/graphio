#ifndef SW_WAYLAND_BACKEND_H
#define SW_WAYLAND_BACKEND_H

#include <wayland-client.h>

#include "include/sw_switcher.h"

struct sw_wayland_backend {
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_seat *seat;
    struct zwlr_foreign_toplevel_manager_v1 *toplevel_manager;
    
    struct sw_switcher *switcher;
};

struct sw_wayland_backend* sw_wayland_backend_create(
    struct wl_display *display, 
    struct sw_switcher *switcher
);

void sw_wayland_backend_sync(struct sw_wayland_backend *self);

void sw_wayland_backend_destroy(struct sw_wayland_backend *self);

#endif