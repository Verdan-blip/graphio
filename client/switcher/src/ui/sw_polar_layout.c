#include <stdlib.h>
#include <string.h>

#include "include/ui/sw_polar_layout.h"

static const double ITEM_SCALE = 0.25;
static const double ITEM_CORNER_RADIUS_SCALE = 0.25;
static const double CORNER_RADIUS_SCALE = 0.18;

static void update_item(
    struct sw_polar_layout_item *item,
    double x, double y,
    double size
) {
    item->size = size;
    item->x = x;
    item->y = y;
    item->corner_radius = size * ITEM_CORNER_RADIUS_SCALE;
}

static struct sw_polar_layout_item* create_item(
    double x, double y,
    double size
) {
    struct sw_polar_layout_item *item = malloc(sizeof(struct sw_polar_layout_item));
    
    update_item(item, x, y, size);

    item->data = NULL;

    return item;
}

static void update_item_positions(
    struct sw_polar_layout *layout
) {
    double inner_padding = layout->inner_paddings;

    double area_size = layout->canvas_size - inner_padding * 2;

    double item_size = area_size * ITEM_SCALE;
    double item_corner_radius = item_size * ITEM_CORNER_RADIUS_SCALE;

    double x, y;

    x = (area_size - item_size) / 2 + inner_padding;
    y = inner_padding;
    update_item(layout->north, x, y, item_size);

    x = (area_size - item_size) / 2 + inner_padding;
    y = area_size + inner_padding - item_size;
    update_item(layout->south, x, y, item_size);

    x = inner_padding;
    y = (area_size - item_size) / 2 + inner_padding;
    update_item(layout->west, x, y, item_size);

    x = area_size + inner_padding - item_size;
    y = (area_size - item_size) / 2 + inner_padding;
    update_item(layout->east, x, y, item_size);

    x = (area_size - item_size) / 2 + inner_padding;
    y = (area_size - item_size) / 2 + inner_padding;
    update_item(layout->center, x, y, item_size);
}

struct sw_polar_layout* sw_polar_layout_create() {
    struct sw_polar_layout *layout = calloc(1, sizeof(struct sw_polar_layout));
    if (!layout) return NULL;

    layout->east = create_item(0, 0, 0);
    layout->west = create_item(0, 0, 0);
    layout->north = create_item(0, 0, 0);
    layout->south = create_item(0, 0, 0);

    layout->center = create_item(0, 0, 0);

    update_item_positions(layout);

    return layout;
}

void sw_polar_layout_set_item_data(
    struct sw_polar_layout *layout, 
    enum sw_polar_item_position pos,
    void* data
) {
    if (pos == SW_POLAR_ITEM_POSITION_EAST) {
        layout->east->data = data;
    } else if (pos == SW_POLAR_ITEM_POSITION_NORTH) {
        layout->north->data = data;
    } else if (pos == SW_POLAR_ITEM_POSITION_SOUTH) {
        layout->south->data = data;
    } else if (pos == SW_POLAR_ITEM_POSITION_WEST) {
        layout->west->data = data;
    }
}

void sw_polar_layout_resize(
    struct sw_polar_layout *layout, 
    double new_size,
    int inner_paddings
) {
    if (!layout) return;
    layout->canvas_size = new_size;
    layout->corner_radius = new_size * CORNER_RADIUS_SCALE;
    layout->inner_paddings = inner_paddings;

    update_item_positions(layout);
}

struct sw_polar_layout_item* sw_polar_layout_find_by_data(
    struct sw_polar_layout *layout, 
    void *data
) {
    if (!layout || !data) return NULL;

    if (layout->north && layout->north->data == data) return layout->north;
    if (layout->south && layout->south->data == data) return layout->south;
    if (layout->west  && layout->west->data  == data) return layout->west;
    if (layout->east  && layout->east->data  == data) return layout->east;

    return NULL;
};

void sw_polar_layout_destroy(struct sw_polar_layout *layout) {
    if (layout) {
        free(layout);
    }
}
