#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include <stdbool.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <wayland-server-core.h>

#include "../generated/contract/protobuf/window_events.pb-c.h"
#include "include/g_toplevel.h"

struct g_window_event {
    struct g_toplevel *toplevel;
    WindowEventType type;
};

struct g_event_manager {
    int server_fd;
    int client_fd;
    struct sockaddr_un addr;
    bool is_running;

    struct wl_event_source *server_source;
};

struct g_event_manager *g_event_manager_create(const char *socket_path);
void g_event_manager_destroy(struct g_event_manager *event_manager);

void g_event_manager_start_events(
    struct g_event_manager *manager, 
    struct wl_display *display
);

void g_event_manager_send(
    struct g_event_manager *manager,
    struct g_window_event event
);

#endif