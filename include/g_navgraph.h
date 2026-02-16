#ifndef G_NAVGRAPH_H
#define G_NAVGRAPH_H

#include <stdbool.h>

#define G_NAVGRAPH_TOPS_COUNT 4

struct g_toplevel;
struct g_workspace_window;

enum g_navgraph_top_dir {
    LEFT = 0, RIGHT = 1, TOP = 2, BOTTOM = 3
};

struct g_navgraph {
    struct g_workspace_window **tops;

    struct g_workspace *guest_slot;

    struct g_workspace_window *current;
};

struct g_navgraph* g_navgraph_create();
void g_navgraph_destroy(struct g_navgraph *navgraph);

bool g_navgraph_is_top_window(struct g_navgraph *navgraph, struct g_workspace_window *window);
bool g_navgraph_is_guest_slot_window(struct g_navgraph *navgraph, struct g_workspace_window *window);

bool g_navgraph_try_promote_guest_window(struct g_navgraph *navgraph, struct g_workspace_window *window);

struct g_workspace_window** g_navgraph_get_all_windows_except(
    struct g_navgraph *navgraph, 
    struct g_workspace_window *except
);

struct g_workspace_window* g_navgraph_find_top_window_by_toplevel(
    struct g_navgraph *navgraph, 
    struct g_toplevel *toplevel,
    enum g_navgraph_top_dir *dir
);

struct g_workspace_window* g_navgraph_find_guest_window_by_toplevel(
    struct g_navgraph *navgraph, 
    struct g_toplevel *toplevel
);

#endif