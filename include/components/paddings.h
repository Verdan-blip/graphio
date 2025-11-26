#ifndef PADDINGS_H
#define PADDINGS_H

struct paddings {
    int start, end;
    int top, bottom;
};

struct paddings create_paddings(int start, int end, int top, int bottom);
struct paddings create_paddings_hv(int horizontal, int vertical);
struct paddings create_paddings_all(int all);

#endif