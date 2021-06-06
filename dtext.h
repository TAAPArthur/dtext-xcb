/* See LICENSE file for copyright and license details. */
#ifndef DTEXT_H
#define DTEXT_H

#include <stdint.h>
#include <xcb/xcb.h>
typedef int32_t dt_error;


typedef struct dt_context dt_context;
typedef struct dt_font dt_font;

typedef struct {
	int32_t x; // Bottom-left of box, relative to point of origin of text
	int32_t y;

	uint32_t w;
	uint32_t h;
} dt_bbox;

typedef struct {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha; // 0 means opaque
} dt_color;

uint16_t get_font_ascent(dt_font* font);
uint16_t get_font_height(dt_font* font);

dt_error dt_init(dt_context **ctx, xcb_connection_t *dpy, xcb_window_t win);
void dt_quit(dt_context *ctx);

dt_error dt_load(dt_context *ctx, dt_font **fnt, char const *name);
void dt_free(dt_context *ctx, dt_font *fnt);

dt_error dt_box(dt_context *ctx, dt_font *fnt, dt_bbox *bbox,
                char const *txt, size_t len);
dt_error dt_draw(dt_context *ctx, dt_font *fnt, dt_color const *color,
                 uint32_t x, uint32_t y, char const *txt, size_t len);


int word_wrap(char *txt);

void dt_draw_all_lines(dt_context *ctx, dt_font *fnt, dt_color const *color,
        uint32_t x, uint32_t starting_y, uint32_t padding, char const *lines, int num_lines);
#endif
