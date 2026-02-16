#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <wayland-util.h>

#include "../include/g_workspace.h"
#include "../include/g_workspace_window.h"

struct g_workspace* g_workspace_create() {
    struct g_workspace *workspace = malloc(sizeof(struct g_workspace));
    workspace->average_score = 0.0f;

    wl_list_init(&workspace->windows);

    return workspace;
}

void g_workspace_destroy(struct g_workspace* workspace) {
    wl_list_remove(&workspace->windows);
    free(workspace);
}

void g_workspace_add_window(struct g_workspace *workspace, struct g_workspace_window *window) {
    struct g_workspace_window *iter;
    wl_list_for_each(iter, &workspace->windows, link) {
        if (window->score > iter->score) {
            wl_list_insert(iter->link.prev, &window->link);
            g_workspace_recalculate_avg_score(workspace);
            return;
        }
    }

    wl_list_insert(workspace->windows.prev, &window->link);
    g_workspace_recalculate_avg_score(workspace);
}

void g_workspace_remove_window(struct g_workspace *workspace, struct g_workspace_window *window) {
    wl_list_remove(&window->link);
    g_workspace_recalculate_avg_score(workspace);
}

void g_workspace_resort(struct g_workspace *workspace, struct g_workspace_window *changed) {
    g_workspace_remove_window(workspace, changed);
    g_workspace_add_window(workspace, changed);
}

void g_workspace_add_penalty_to_all(
    struct g_workspace *workspace, 
    struct g_workspace_window *except,
    uint64_t now
) {
    struct g_workspace_window *window;
    wl_list_for_each(window, &workspace->windows, link) {
        if (window != except) {
            g_workspace_window_add_penalty(window, now);
        }
    }
}

void g_workspace_recalculate_avg_score(struct g_workspace *workspace) {
    struct g_workspace_window *window;

    double new_avg_score = 0;
    wl_list_for_each(window, &workspace->windows, link) {
        if (window->toplevel->focused) {
            window->score += 1;
            window->last_usage = time(NULL);
        }

        new_avg_score += window->score;
    }

    int window_count = wl_list_length(&workspace->windows);

    if (window_count != 0) workspace->average_score = new_avg_score / window_count;
}

bool g_workspace_is_empty(struct g_workspace *workspace) {
    return wl_list_empty(&workspace->windows);
}

bool g_workspace_contains_window(struct g_workspace *workspace, struct g_workspace_window *window) {
    if (g_workspace_is_empty(workspace)) return false;

    struct g_workspace_window *workspace_window = NULL;
    wl_list_for_each(window, &workspace->windows, link) {
        if (workspace_window == window) return true;
    }

    return false;
}

struct g_workspace_window* g_workspace_poll_first(struct g_workspace *workspace) {
    struct wl_list *first = workspace->windows.next;
    struct g_workspace_window *window = wl_container_of(first, window, link);

    wl_list_remove(first);

    return window;
}

struct g_workspace_window* g_workspace_get_window_by_toplevel(
    struct g_workspace *workspace, 
    struct g_toplevel *toplevel
) {
    struct g_workspace_window *window;
    wl_list_for_each(window, &workspace->windows, link) {
        if (window->toplevel == toplevel) return window;
    }

    return NULL;
}
