#ifndef G_NAVIGATOR_H
#define G_NAVIGATOR_H

#include "../include/g_navgraph.h"

struct g_toplevel;

struct g_navigator {
    struct g_navgraph *navgraph;
};

struct g_navigator* g_navigator_create();
void g_navigator_destroy(struct g_navigator *navigator);

void g_navigator_on_create_toplevel(struct g_navgraph *navgraph, struct g_toplevel *toplevel);
void g_navigator_on_destroy_toplevel(struct g_navgraph *navgraph, struct g_toplevel *toplevel);
void g_navigator_on_focus_toplevel(struct g_navgraph *navgraph, struct g_toplevel *toplevel);

#endif