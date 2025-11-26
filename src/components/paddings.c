#include "../include/components/paddings.h"

struct paddings create_paddings(int start, int end, int top, int bottom) {
    return (struct paddings) {
        .start = start,
        .end = end,
        .top = top,
        .bottom = bottom
    };
}

struct paddings create_paddings_hv(int horizontal, int vertical) {
    return create_paddings(horizontal, horizontal, vertical, vertical);
}

struct paddings create_paddings_all(int all) {
    return create_paddings_hv(all, all);
}