#ifndef G_WORKSPACE_WINDOW_H
#define G_WORKSPACE_WINDOW_H

#include <stdint.h>
#include <wayland-util.h>

#include "../include/g_server.h"

struct g_workspace_window {
    struct wl_list link;

    struct g_toplevel *toplevel;

    double score;
    uint64_t last_usage;
};

struct g_workspace_window* g_workspace_window_create(struct g_toplevel *toplevel);
void g_workspace_window_destroy(struct g_workspace_window* workspace_window);

void g_workspace_window_add_penalty(struct g_workspace_window* workspace_window, uint64_t now);
void g_workspace_window_add_reward(struct g_workspace_window* workspace_window, uint64_t now);

#endif