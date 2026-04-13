#define _POSIX_C_SOURCE 200809L

#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/stat.h>

#include "wlr/util/log.h"
#include <wayland-util.h>

#include "include/events/g-event-manager.h"
#include "include/g_server.h"

static double get_now() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static int handle_client_connection(int fd, uint32_t mask, void *data) {
    struct g_event_manager *manager = data;
    struct g_server *server = manager->server;

    int client_fd = accept(fd, NULL, NULL);
    if (client_fd < 0) {
        wlr_log(WLR_ERROR, "g-event-manager: failed to accept");
        return 1;
    }

    if (manager->client_fd >= 0) {
        wlr_log(WLR_INFO, "g-event-manager: dropping old client for a new one");
        close(manager->client_fd);
    }

    manager->client_fd = client_fd;
    wlr_log(WLR_INFO, "g-event-manager: Python-client connected (fd: %d)", client_fd);

    // Send all window events
    struct g_toplevel *toplevel;
    wl_list_for_each(toplevel, &server->toplevels, link) {
        struct g_window_event event = {
			.toplevel = toplevel,
			.type = WINDOW_EVENT_TYPE__WINDOW_EVENT_TYPE_CREATE
		};

		g_event_manager_send(toplevel->server->event_manager, event);

        if (toplevel->focused) {
            struct g_window_event event = {
                .toplevel = toplevel,
                .type = WINDOW_EVENT_TYPE__WINDOW_EVENT_TYPE_FOCUS_IN
            };

            g_event_manager_send(server->event_manager, event);
        }
    }

    return 1;
}

struct g_event_manager *g_event_manager_create(
    struct g_server *server, 
    const char *socket_path
) {
    if (socket_path == NULL) return NULL;

    struct g_event_manager *manager = calloc(1, sizeof(struct g_event_manager));
    if (!manager) return NULL;

    manager->client_fd = -1;
    manager->server_fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    
    if (manager->server_fd < 0) {
        wlr_log(WLR_ERROR, "g-event-manager: failed to create unix socket");
        free(manager);
        return NULL;
    }

    manager->addr.sun_family = AF_UNIX;
    strncpy(
        manager->addr.sun_path, 
        socket_path, 
        sizeof(manager->addr.sun_path) - 1
    );

    unlink(socket_path); 

    manager->server = server;

    return manager;
}

void g_event_manager_start_events(
    struct g_event_manager *manager, 
    struct wl_display *display
) {
    if (!manager) return;

    if (manager->server_fd < 0) {
        manager->server_fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    }

    unlink(manager->addr.sun_path);

    if (bind(manager->server_fd, (struct sockaddr *)&manager->addr, sizeof(manager->addr)) == -1) {
        wlr_log(WLR_ERROR, "g-event-manager: bind failed: %s", strerror(errno));
        return;
    }

    if (listen(manager->server_fd, 5) == -1) {
        wlr_log(WLR_ERROR, "g-event-manager: listen failed");
        return;
    }

    chmod(manager->addr.sun_path, 0666);

    struct wl_event_loop *loop = wl_display_get_event_loop(display);

    manager->server_source = wl_event_loop_add_fd(loop, 
        manager->server_fd, 
        WL_EVENT_READABLE, 
        handle_client_connection,
        manager
    );

    if (manager->server_source) {
        wlr_log(WLR_INFO, "g-event-manager: SUCCESSFULLY registered on FD %d", manager->server_fd);
    } else {
        wlr_log(WLR_ERROR, "g-event-manager: FAILED to register FD");
    }

    manager->is_running = true;
}

void g_event_manager_destroy(struct g_event_manager *manager) {
    if (!manager) return;

    if (manager->server_source) {
        wl_event_source_remove(manager->server_source);
    }

    if (manager->client_fd >= 0) close(manager->client_fd);
    if (manager->server_fd >= 0) close(manager->server_fd);

    unlink(manager->addr.sun_path);
    free(manager);
}

void g_event_manager_send(
    struct g_event_manager *manager,
    struct g_window_event event
) {
    if (!manager) return;
    if (!manager->is_running) return;
    if (manager->client_fd < 0) return;
    if (!event.toplevel) return;

    WindowEvent msg = WINDOW_EVENT__INIT;
    msg.type = event.type;
    msg.win_id = event.toplevel->id;
    msg.app_id = (char *)(event.toplevel->app_id);
    msg.timestamp = get_now();

    size_t len = window_event__get_packed_size(&msg);
    uint8_t *buf = malloc(len);
    if (!buf) {
        wlr_log(WLR_ERROR, "g-event-manager: malloc failed");
        return;
    }
    
    window_event__pack(&msg, buf);

    uint32_t net_len = htonl((uint32_t)len);
    ssize_t sent_len = send(manager->client_fd, &net_len, sizeof(net_len), 0);

    if (sent_len != -1) {
        ssize_t sent_body = send(manager->client_fd, buf, len, 0);
        
        if (sent_body == -1) {
            wlr_log(WLR_ERROR, "g-event-manager: body send failed");
            if (errno == EPIPE || errno == ECONNRESET) {
                close(manager->client_fd);
                manager->client_fd = -1;
            }
        }
    } else {
        wlr_log(WLR_ERROR, "g-event-manager: header send failed");
        if (errno == EPIPE || errno == ECONNRESET) {
            close(manager->client_fd);
            manager->client_fd = -1;
        }
    }

    free(buf);
}