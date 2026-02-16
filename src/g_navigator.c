#include "../include/g_navigator.h"
#include "../include/g_workspace_window.h"
#include "../include/g_server.h"
#include "../include/g_navgraph.h"
#include "../include/g_workspace.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <wayland-util.h>

struct g_navigator* g_navigator_create() {
    struct g_navigator *navigator = malloc(sizeof(struct g_navigator));
    navigator->navgraph = g_navgraph_create();

    return navigator;
}

void g_navigator_destroy(struct g_navigator *navigator) {
    g_navgraph_destroy(navigator->navgraph);
    free(navigator);
}

void g_navigator_on_create_toplevel(
    struct g_navgraph *navgraph,
    struct g_toplevel *toplevel
) {
    struct g_workspace_window *window = g_workspace_window_create(toplevel);

    for (int i = 0; i < G_NAVGRAPH_TOPS_COUNT; i++) {
        if (!navgraph->tops[i]) {
            navgraph->tops[i] = window;
            return;
        }
    }

    g_workspace_add_window(navgraph->guest_slot, window);
}

void g_navigator_on_destroy_toplevel(
    struct g_navgraph *navgraph,
    struct g_toplevel *toplevel
) {
    enum g_navgraph_top_dir dir;
    struct g_workspace_window *top = g_navgraph_find_top_window_by_toplevel(navgraph, toplevel, &dir);

    if (top) {
        g_workspace_window_destroy(top);
        navgraph->tops[dir] = NULL;

        // promotion из guest
        if (!g_workspace_is_empty(navgraph->guest_slot)) {
            struct g_workspace_window *best =
                g_workspace_poll_first(navgraph->guest_slot);

            navgraph->tops[dir] = best;
        }

        return;
    }

    struct g_workspace_window *guest = g_navgraph_find_guest_window_by_toplevel(navgraph, toplevel);

    if (guest) {
        g_workspace_remove_window(navgraph->guest_slot, guest);
        g_workspace_window_destroy(guest);
    }
}

void g_navigator_on_focus_toplevel(
    struct g_navgraph *navgraph,
    struct g_toplevel *toplevel
) {
    uint64_t now = time(NULL);

    enum g_navgraph_top_dir dir;
    struct g_workspace_window *window = g_navgraph_find_top_window_by_toplevel(navgraph, toplevel, &dir);

    if (window) {
        navgraph->current = window;

        // Reward
        g_workspace_window_add_reward(window, now);

        // Penalty
        for (int i = 0; i < G_NAVGRAPH_TOPS_COUNT; i++) {
            if (navgraph->tops[i] && navgraph->tops[i] != window) {
                g_workspace_window_add_penalty(navgraph->tops[i], now);
            }
        }

        g_workspace_add_penalty_to_all(navgraph->guest_slot, NULL, now);

        return;
    }

    window = g_navgraph_find_guest_window_by_toplevel(navgraph, toplevel);

    if (window) {
        navgraph->current = window;

        // Reward
        g_workspace_window_add_reward(window, now);

        // Penalty
        for (int i = 0; i < G_NAVGRAPH_TOPS_COUNT; i++) {
            if (navgraph->tops[i]) {
                g_workspace_window_add_penalty(navgraph->tops[i], now);
            }
        }

        g_workspace_add_penalty_to_all(navgraph->guest_slot, window, now);

        g_navgraph_try_promote_guest_window(navgraph, window);
    }
}

bool g_navgraph_try_promote_guest_window(struct g_navgraph *navgraph, struct g_workspace_window *window) {
    if (g_workspace_contains_window(navgraph->guest_slot, window)) {
        int worst_dir = -1;
        double worst_score = INFINITY;

        for (int i = 0; i < G_NAVGRAPH_TOPS_COUNT; i++) {
            if (navgraph->tops[i]->score < worst_score) {
                worst_score = navgraph->tops[i]->score;
                worst_dir = i;
            }
        }

        if (worst_dir >= 0 && window->score > worst_score * 1.15) {
            struct g_workspace_window *demoted = navgraph->tops[worst_dir];

            g_workspace_add_window(navgraph->guest_slot, demoted);
            g_workspace_remove_window(navgraph->guest_slot, window);

            navgraph->tops[worst_dir] = window;

            return true;
        }
    }

    return false;
}
