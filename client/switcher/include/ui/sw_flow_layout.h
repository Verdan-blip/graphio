#ifndef SW_FLOW_LAYOUT_H
#define SW_FLOW_LAYOUT_H

#include "include/math/sw_vec2.h"

struct sw_flow_item {
    struct sw_vec2 pos;
    struct sw_vec2 size;
    double corner_radius;

    void *data;
};

struct sw_flow_layout {
    double canvas_size;
    double max_width;

    struct sw_vec2 item_size;

    double inner_padding;
    double gap;
    double corner_radius;

    struct sw_flow_item **items;
    int items_count;
    int items_capacity;

    int rows, columns;

    double total_height;
};

struct sw_flow_layout* sw_flow_layout_create();

void sw_flow_layout_add_item(struct sw_flow_layout *layout, void *data);
void sw_flow_layout_remove_item(struct sw_flow_layout *layout, void *data);

struct sw_flow_item* sw_flow_layout_find_by_data(
    struct sw_flow_layout *layout, 
    void *data
);

void sw_flow_layout_clear(struct sw_flow_layout *layout);

void sw_flow_layout_resize(
    struct sw_flow_layout *layout, 
    double max_width, 
    struct sw_vec2 item_size, 
    double inner_paddings,
    double gap
);

void sw_flow_layout_destroy(struct sw_flow_layout *layout);

#endif