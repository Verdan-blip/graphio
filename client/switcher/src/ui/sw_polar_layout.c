#include <stdlib.h>
#include <string.h>

#include "include/ui/sw_polar_layout.h"
#include "include/math/sw_vec2.h"

static const double ITEM_SCALE = 0.25;
static const double ITEM_CORNER_RADIUS_SCALE = 0.25;
static const double CORNER_RADIUS_SCALE = 0.18;
static const double ITEM_CENTER_SCALE = 0.45;

static void update_item(
    struct sw_polar_layout_item *item,
    struct sw_vec2 pos,
    struct sw_vec2 size
) {
    item->size = size;
    item->pos = pos;
    item->corner_radius = size.y * ITEM_CORNER_RADIUS_SCALE;
}

static struct sw_polar_layout_item* create_item(
    struct sw_vec2 pos,
    struct sw_vec2 size
) {
    struct sw_polar_layout_item *item = malloc(sizeof(struct sw_polar_layout_item));
    
    update_item(item, pos, size);

    item->data = NULL;

    return item;
}

static void update_item_positions(
    struct sw_polar_layout *layout
) {
    double inner_padding = layout->inner_paddings;

    struct sw_vec2 area_size = sw_vec2_create(
        layout->canvas_size.x - inner_padding * 2,
        layout->canvas_size.y - inner_padding * 2
    );

    struct sw_vec2 item_size = sw_vec2_mul(area_size, ITEM_SCALE);

    double item_corner_radius = item_size.y * ITEM_CORNER_RADIUS_SCALE;

    struct sw_vec2 pos;

    pos.x = (area_size.x - item_size.x) / 2 + inner_padding;
    pos.y = inner_padding;
    update_item(layout->north, pos, item_size);

    pos.x = (area_size.x - item_size.x) / 2 + inner_padding;
    pos.y = area_size.y + inner_padding - item_size.y;
    update_item(layout->south, pos, item_size);

    pos.x = inner_padding;
    pos.y = (area_size.x - item_size.x) / 2 + inner_padding;
    update_item(layout->west, pos, item_size);

    pos.x = area_size.x + inner_padding - item_size.x;
    pos.y = (area_size.x - item_size.x) / 2 + inner_padding;
    update_item(layout->east, pos, item_size);

    struct sw_vec2 center_item_size = sw_vec2_mul(item_size, ITEM_CENTER_SCALE);
    pos.x = (area_size.x - center_item_size.x) / 2 + inner_padding;
    pos.y = (area_size.y - center_item_size.y) / 2 + inner_padding;
    update_item(layout->center, pos, center_item_size);
}

struct sw_polar_layout* sw_polar_layout_create() {
    struct sw_polar_layout *layout = calloc(1, sizeof(struct sw_polar_layout));
    if (!layout) return NULL;

    layout->east = create_item(sw_vec2_empty(), sw_vec2_empty());
    layout->west = create_item(sw_vec2_empty(), sw_vec2_empty());
    layout->north = create_item(sw_vec2_empty(), sw_vec2_empty());
    layout->south = create_item(sw_vec2_empty(), sw_vec2_empty());

    layout->center = create_item(sw_vec2_empty(), sw_vec2_empty());

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
    struct sw_vec2 new_size,
    int inner_paddings
) {
    if (!layout) return;
    layout->canvas_size = new_size;
    layout->corner_radius = new_size.y * CORNER_RADIUS_SCALE;
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
