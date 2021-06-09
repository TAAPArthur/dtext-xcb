/* See LICENSE file for copyright and license details. */

#include <assert.h>
#include <stdint.h>
#include <wchar.h>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include FT_ADVANCES_H
#include <xcb/xcb.h>
#include <xcb/render.h>
#include <xcb/xcb_renderutil.h>

#include "../dtext.h"

#define TEXT "The quick brown fox jumps over the lazy dog. "
#define TEXT2 "AAA\nBB\nC\nDD\nEEE\nFFFF"
//#define FONT "/usr/share/fonts/fantasque-sans-mono/FantasqueSansMono-Regular.otf:16"
#define FONT "/usr/share/fonts/TTF/LiberationMono-Regular.ttf:48"
//#define FONT "/usr/share/fonts/libertine/LinLibertine_R.otf:16"

xcb_connection_t *dis;
xcb_window_t root;
xcb_window_t win;
xcb_gcontext_t gc;

dt_context *ctx;
dt_font *fnt;
dt_color color;
dt_color color_inv;

static void setup_x() {
    dis = xcb_connect(NULL, NULL);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator (xcb_get_setup (dis));
    xcb_screen_t* screen = iter.data;
    root = screen->root;

    win = xcb_generate_id(dis);
    uint32_t mask[] = {XCB_EVENT_MASK_EXPOSURE};
    xcb_create_window(dis, XCB_COPY_FROM_PARENT, win, screen->root, 0, 0, 500, 500, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual,
            XCB_CW_EVENT_MASK, &mask );
    xcb_map_window(dis, win);


    gc = xcb_generate_id(dis);
    xcb_create_gc (dis, gc, win, XCB_GC_FOREGROUND, &screen->white_pixel);
}

static void setup_dt() {
    memset(&color, 0, sizeof(color));
    color.blue = 0xFF;
    color.alpha = 0xFF;
    memset(&color_inv, 0, sizeof(color_inv));
    color_inv.red = 0xFF;
    color_inv.green = 0x00;
    color_inv.blue = 0xFF;
    color_inv.alpha = 0xFF;
}

static void draw() {
    dt_bbox bbox;

    assert(!dt_draw(ctx, fnt, &color, 10, 50, TEXT, strlen(TEXT)));

    assert(!dt_box(dis, fnt, &bbox, TEXT, strlen(TEXT)));

    xcb_poly_fill_rectangle(dis, win, gc, 1, (xcb_rectangle_t[1]){{10 + bbox.x, 100 + bbox.y, bbox.w, bbox.h}});
    assert(!dt_draw(ctx, fnt, &color_inv, 10, 100, TEXT, strlen(TEXT)));

    xcb_poly_fill_rectangle(dis, win, gc, 1, (xcb_rectangle_t[1]){{10 + bbox.x, 150 - dt_get_font_ascent(fnt), bbox.w, dt_get_font_height(fnt)}});
    assert(!dt_draw(ctx, fnt, &color_inv, 10, 150, TEXT, strlen(TEXT)));

    char buffer[32] = TEXT2;
    int num_lines = dt_split_lines(buffer);
    int offset = dt_draw_all_lines(ctx, fnt, &color_inv, 10, 200, 10, buffer, num_lines);

    char buffer2[255] = TEXT;
    num_lines = dt_word_wrap_n(dis, fnt, buffer2, 1, 400);
    dt_draw_all_lines(ctx, fnt, &color_inv, 10, offset, 10, buffer2, num_lines);
    xcb_flush(dis);
}

int main() {
    setup_x();
    assert(!dt_init_context(&ctx, dis, win));
    assert(!dt_load_font(dis, &fnt, FONT));
    setup_dt();

    draw();

    xcb_generic_event_t* event;
    while((event=xcb_wait_for_event(dis))) {
        draw();
        free(event);
    }

    dt_free_font(dis, fnt);
    dt_free_context(ctx);
    xcb_disconnect(dis);
}
