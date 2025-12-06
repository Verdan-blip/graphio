#ifndef G_TOPLEVEL_INTERACTION_H
#define G_TOPLEVEL_INTERACTION_H

#include "../include/g-toplevel.h"

struct g_toplevel_interaction {
    struct g_toplevel *grabbed_toplevel;
    int grab_pos_x, grab_pos_y;
    uint32_t resize_edges;
};

struct g_toplevel_interaction* g_toplevel_interaction_create();
void g_toplevel_interaction_destroy(struct g_toplevel_interaction *interaction);

#endif