#include <stddef.h>
#include <stdlib.h>
#include <wayland-util.h>

#include "../include/g_navgraph.h"
#include "../include/g_workspace.h"
#include "../include/g_server.h"
#include "../include/g_workspace_window.h"

struct g_navgraph* g_navgraph_create() {
    struct g_navgraph *navgraph = malloc(sizeof(struct g_navgraph));

    navgraph->tops = malloc(sizeof(struct g_workspace_window) * G_NAVGRAPH_TOPS_COUNT);
    for (int i = 0; i < G_NAVGRAPH_TOPS_COUNT; ++i) {
        navgraph->tops[i] = NULL;
    }

    navgraph->guest_slot = g_workspace_create();

    return navgraph;
}

void g_navgraph_destroy(struct g_navgraph *navgraph) {
    for (int i = 0; i < G_NAVGRAPH_TOPS_COUNT; ++i) {
        g_workspace_window_destroy(navgraph->tops[i]);
    }
    free(navgraph->tops);

    g_workspace_destroy(navgraph->guest_slot);

    free(navgraph);
}

bool g_navgraph_is_top_window(struct g_navgraph *navgraph, struct g_workspace_window *window) {
    for (int i = 0; i < G_NAVGRAPH_TOPS_COUNT; ++i) {
        if (navgraph->tops[i] == window) return true;
    }

    return false;
}

bool g_navgraph_is_guest_slot_window(struct g_navgraph *navgraph, struct g_workspace_window *window) {
    return g_workspace_contains_window(navgraph->guest_slot, window);
}

struct g_workspace_window* g_navgraph_find_top_window_by_toplevel(
    struct g_navgraph *navgraph, 
    struct g_toplevel *toplevel,
    enum g_navgraph_top_dir *dir
) {
    for (int i = 0; i < G_NAVGRAPH_TOPS_COUNT; ++i) {
        if (navgraph->tops[i]->toplevel == toplevel) {
            (*dir) = i;
            return navgraph->tops[i];
        }
    }

    return NULL;
}

struct g_workspace_window* g_navgraph_find_guest_window_by_toplevel(
    struct g_navgraph *navgraph, 
    struct g_toplevel *toplevel
) {
    return g_workspace_get_window_by_toplevel(navgraph->guest_slot, toplevel);
}

