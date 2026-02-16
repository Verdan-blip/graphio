#ifndef G_WORKSPACE_H
#define G_WORKSPACE_H

#include <stdbool.h>
#include <stdint.h>
#include <wayland-util.h>

struct g_toplevel;
struct g_workspace_window;

struct g_workspace {
    struct wl_list windows;

    double average_score;
};

struct g_workspace* g_workspace_create();
void g_workspace_focus_changed(struct g_workspace* workspace);
void g_workspace_destroy(struct g_workspace* workspace);

void g_workspace_add_window(struct g_workspace *workspace, struct g_workspace_window *window);
void g_workspace_remove_window(struct g_workspace *workspace, struct g_workspace_window *window);
void g_workspace_resort(struct g_workspace *workspace, struct g_workspace_window *changed);

void g_workspace_add_penalty_to_all(
    struct g_workspace *workspace, 
    struct g_workspace_window *except,
    uint64_t now
);

// Utils
void g_workspace_recalculate_avg_score(struct g_workspace *workspace);
bool g_workspace_is_empty(struct g_workspace *workspace);
bool g_workspace_contains_window(struct g_workspace *workspace, struct g_workspace_window *window);
struct g_workspace_window* g_workspace_poll_first(struct g_workspace *workspace);

// wlroots-specific utils
struct g_workspace_window* g_workspace_get_window_by_toplevel(struct g_workspace *workspace, struct g_toplevel *toplevel);

#endif