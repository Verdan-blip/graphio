#ifndef SW_POLAR_LAYOUT_H
#define SW_POLAR_LAYOUT_H

#include <stdbool.h>

enum sw_polar_item_position {
    SW_POLAR_ITEM_POSITION_WEST,
    SW_POLAR_ITEM_POSITION_EAST,
    SW_POLAR_ITEM_POSITION_NORTH,
    SW_POLAR_ITEM_POSITION_SOUTH,
};

struct sw_polar_layout_item {
    double size;
    double x, y;
    int corner_radius;
    void *data;
};

struct sw_polar_layout {
    double canvas_size;

    int inner_paddings;
    int corner_radius;

    int padding_between_items;

    struct sw_polar_layout_item *north;
    struct sw_polar_layout_item *south;
    struct sw_polar_layout_item *west;
    struct sw_polar_layout_item *east;

    struct sw_polar_layout_item *center;
};

struct sw_polar_layout* sw_polar_layout_create();

void sw_polar_layout_destroy(struct sw_polar_layout *layout);

void sw_polar_layout_set_item_data(
    struct sw_polar_layout *layout, 
    enum sw_polar_item_position pos,
    void* data
);

struct sw_polar_layout_item* sw_polar_layout_find_by_data(
    struct sw_polar_layout *layout, 
    void *data
);

void sw_polar_layout_resize(
    struct sw_polar_layout *layout, 
    double new_size,
    int inner_paddings
);

#endif