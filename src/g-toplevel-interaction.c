#include <stdlib.h>
#include "../include/g-toplevel-interaction.h"

struct g_toplevel_interaction* g_toplevel_interaction_create() {
    struct g_toplevel_interaction *interaction = calloc(1, sizeof(struct g_toplevel_interaction));
    interaction->grab_pos_x = 0;
    interaction->grab_pos_y = 0;
    interaction->grabbed_toplevel = NULL;

    return interaction;
}

void g_toplevel_interaction_destroy(struct g_toplevel_interaction *interaction) {
    free(interaction);
}