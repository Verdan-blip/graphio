#include <stdlib.h>

#include "include/ui/sw_flow_layout.h"
#include "include/math/sw_vec2.h"

static const double CORNER_RADIUS_SCALE = 0.05;
static const double ITEM_CORNER_RADIUS_SCALE = 0.25;

static void flow_layout_update(struct sw_flow_layout *layout) {
    if (layout->items_count == 0) {
        layout->rows = 0;
        layout->columns = 0;
        layout->total_height = 0;
        return;
    }

    double available_width = layout->max_width - (layout->inner_padding * 2);
    
    int cols = (int) ((available_width + layout->gap) / (layout->item_size.x + layout->gap));
    if (cols < 1) cols = 1;
    if (cols > layout->items_count) cols = layout->items_count;

    layout->columns = cols;
    layout->rows = (layout->items_count + cols - 1) / cols;

    double content_width = (cols * layout->item_size.x) + ((cols - 1) * layout->gap);
    
    double horizontal_offset = (available_width - content_width) / 2.0;

    for (int i = 0; i < layout->items_count; i++) {
        int row = i / cols;
        int col = i % cols;


        int items_in_this_row = cols;
        if (row == layout->rows - 1) {
            items_in_this_row = layout->items_count % cols;
            if (items_in_this_row == 0) items_in_this_row = cols;
        }
        
        double row_content_width = (items_in_this_row * layout->item_size.x) + ((items_in_this_row - 1) * layout->gap);
        double row_offset = (available_width - row_content_width) / 2.0;


        layout->items[i]->pos.x = layout->inner_padding + row_offset + col * (layout->item_size.x + layout->gap);
        layout->items[i]->pos.y = layout->inner_padding + row * (layout->item_size.y + layout->gap);

        layout->items[i]->size = layout->item_size;
        layout->items[i]->corner_radius = layout->item_size.x * ITEM_CORNER_RADIUS_SCALE;
    }

    layout->total_height = (layout->rows * layout->item_size.y) + 
                           ((layout->rows - 1) * layout->gap) + 
                           (layout->inner_padding * 2);
}

struct sw_flow_layout* sw_flow_layout_create() {
    struct sw_flow_layout *layout = calloc(1, sizeof(struct sw_flow_layout));

    layout->items_count = 0;
    layout->items_capacity = 8;
    layout->items = malloc(sizeof(struct sw_flow_item*) * layout->items_capacity);

    layout->rows = 0;
    layout->columns = 0;
    layout->total_height = 0;

    return layout;
}

void sw_flow_layout_resize(
    struct sw_flow_layout *layout, 
    double max_width, 
    struct sw_vec2 item_size, 
    double inner_paddings,
    double gap
) {
    layout->max_width = max_width;
    layout->item_size = item_size;
    layout->corner_radius = max_width * CORNER_RADIUS_SCALE;
    layout->inner_padding = inner_paddings;
    layout->gap = gap;

    flow_layout_update(layout);
}

void sw_flow_layout_add_item(struct sw_flow_layout *layout, void *data) {
    if (layout->items_count >= layout->items_capacity) {
        layout->items_capacity *= 2;
        layout->items = realloc(layout->items, sizeof(struct sw_flow_item*) * layout->items_capacity);
    }

    struct sw_flow_item *item = malloc(sizeof(struct sw_flow_item));
    item->size = layout->item_size;
    item->corner_radius = layout->item_size.x * ITEM_CORNER_RADIUS_SCALE;
    item->data = data;
    
    layout->items[layout->items_count++] = item;
    flow_layout_update(layout);
}

void sw_flow_layout_remove_item(struct sw_flow_layout *layout, void *data) {
    if (!layout || !data) return;

    int index_to_remove = -1;

    for (int i = 0; i < layout->items_count; i++) {
        if (layout->items[i]->data == data) {
            index_to_remove = i;
            break;
        }
    }

    if (index_to_remove == -1) return;

    free(layout->items[index_to_remove]);

    for (int i = index_to_remove; i < layout->items_count - 1; i++) {
        layout->items[i] = layout->items[i + 1];
    }

    layout->items_count--;

    flow_layout_update(layout);
}

struct sw_flow_item* sw_flow_layout_find_by_data(struct sw_flow_layout *layout, void *data) {
    if (!layout || !data) return NULL;

    for (int i = 0; i < layout->items_count; i++) {
        if (layout->items[i] && layout->items[i]->data == data) {
            return layout->items[i];
        }
    }

    return NULL;
}

void sw_flow_layout_clear(struct sw_flow_layout *layout) {
    if (!layout) return;

    for (int i = 0; i < layout->items_count; i++) {
        if (layout->items[i]) {
            free(layout->items[i]);
            layout->items[i] = NULL;
        }
    }
    layout->items_count = 0;
    layout->rows = 0;
    layout->columns = 0;
    layout->total_height = 0;
}

void sw_flow_layout_destroy(struct sw_flow_layout *layout) {
    if (!layout) return;
    for (int i = 0; i < layout->items_count; i++) {
        free(layout->items[i]);
    }
    free(layout->items);
    free(layout);
}
