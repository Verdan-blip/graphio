#include <string.h>
#include <stdlib.h>

#include "include/wayland/sw_wayland_backend.h"
#include "include/sw_toplevel.h"

static void registry_handle_global(
    void *data, 
    struct wl_registry *registry, 
    uint32_t name, 
    const char *interface, 
    uint32_t version
) {
    struct sw_wayland_backend *wayland_backend = data;

    if (strcmp(interface, zwlr_foreign_toplevel_manager_v1_interface.name) == 0) {
        wayland_backend->toplevel_manager = wl_registry_bind(
            registry, 
            name, 
            &zwlr_foreign_toplevel_manager_v1_interface, 
            1
        );
        
        static const struct zwlr_foreign_toplevel_manager_v1_listener manager_impl = {
            .toplevel = sw_handle_toplevel
        };

        zwlr_foreign_toplevel_manager_v1_add_listener(wayland_backend->toplevel_manager, &manager_impl, wayland_backend->switcher);
    }

    if (strcmp(interface, wl_seat_interface.name) == 0) {
        wayland_backend->seat = wl_registry_bind(registry, name, &wl_seat_interface, version);
    }
}

static void registry_handle_global_remove(
    void *data, 
    struct wl_registry *registry, 
    uint32_t name
) {
    
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

struct sw_wayland_backend* sw_wayland_backend_create(
    struct wl_display *display, 
    struct sw_switcher *switcher
) {
    struct sw_wayland_backend *wayland_backend = calloc(1, sizeof(struct sw_wayland_backend));
    wayland_backend->display = display;
    wayland_backend->switcher = switcher;
    wayland_backend->registry = wl_display_get_registry(display);

    switcher->wayland_backend = wayland_backend;

    wl_registry_add_listener(wayland_backend->registry, &registry_listener, wayland_backend);

    return wayland_backend;
}

void sw_wayland_backend_sync(struct sw_wayland_backend *self) {
    wl_display_roundtrip(self->display);
}

void sw_wayland_backend_destroy(struct sw_wayland_backend *wayland_backend) {
    if (!wayland_backend) return;
    if (wayland_backend->toplevel_manager) zwlr_foreign_toplevel_manager_v1_destroy(wayland_backend->toplevel_manager);
    if (wayland_backend->registry) wl_registry_destroy(wayland_backend->registry);
    if (wayland_backend->seat) wl_seat_destroy(wayland_backend->seat);
    
    free(wayland_backend);
}