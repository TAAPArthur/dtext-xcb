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

uint16_t dt_get_font_ascent(dt_font* font);
uint16_t dt_get_font_height(dt_font* font);

dt_error dt_init_context(dt_context **ctx, xcb_connection_t *dpy, xcb_window_t win);
void dt_free_context(dt_context *ctx);

dt_error dt_load_font(xcb_connection_t *dis, dt_font **res, char const *name, int size);
dt_error dt_load_fonts(xcb_connection_t *dis, dt_font **res, char const *name, int n, int size);
void dt_free_font(xcb_connection_t *dis, dt_font *fnt);

dt_error dt_box(xcb_connection_t *dis, dt_font *fnt, dt_bbox *bbox,
                char const *txt, size_t len);
dt_error dt_draw(dt_context *ctx, dt_font *fnt, dt_color const *color,
                 uint32_t x, uint32_t y, char const *txt, size_t len);

uint16_t dt_get_text_width(xcb_connection_t* dis, dt_font *fnt, char const *txt, size_t len) ;

/**
 * Splits txt into N lines by by total length
 *
 * Breaks up text into N lines such that strings are generally less than width pixels
 * @return N, the new number of strings
 */
int dt_word_wrap_n(xcb_connection_t* dis, dt_font *fnt, char *txt, int num_lines, uint32_t width);

/**
 * Replace all instances of '\n' with \0.
 *
 * Split txt into N seperate string by replacing the new line marker
 * with the null string.
 * @return N, the new number of strings
 */
int dt_split_lines(char *txt);

/**
 * Splits txt into N lines by '\n' and by total length
 *
 * Breaks up text into N lines such that each human line is its own string
 * and string are generally less than width pixels
 * @return N, the new number of strings
 */
int dt_word_wrap_line(xcb_connection_t*dis, dt_font *fnt, char *txt, uint32_t width);

int dt_draw_all_lines(dt_context *ctx, dt_font *fnt, dt_color const *color,
        uint32_t x, uint32_t starting_y, uint32_t padding, char const *lines, int num_lines);
#endif
