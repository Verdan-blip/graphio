#ifndef G_ICONS_H
#define G_ICONS_H

#include <wlr/render/wlr_texture.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_output.h>
#include <wlr/util/transform.h>

struct tinywl_output;

struct g_icon {
    int width, height;
    struct wlr_texture *texture;
};

struct g_icon* g_icon_load_from_path(const char *path, struct wlr_renderer *render);
void g_icon_destroy(struct g_icon *icon);

struct g_icon* g_icon_create_empty(int width, int height, struct wlr_renderer *renderer);

struct g_icon* g_icon_load_from_path_or_default(
    const char *path, 
    int width, 
    int height, 
    struct wlr_renderer *renderer
);

void g_icon_on_render_pass(
    struct g_icon *icon, 
    int pos_x, int pos_y,
    struct wlr_render_pass *pass
);

#endif