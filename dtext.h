/* See LICENSE file for copyright and license details. */
#ifndef DTEXT_H
#define DTEXT_H

#include <stdint.h>
#include <xcb/xcb.h>


typedef struct dt_context dt_context;
typedef struct dt_font dt_font;

typedef struct {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha; // 0 means opaque
} dt_color;

/**
 * returns the height of the highest character of the font, relative to the baseline
 */
uint16_t dt_get_font_ascent(dt_font* font);
/**
 * returns the total height of the highest character in the font.
 */
uint16_t dt_get_font_height(dt_font* font);

/*
 * Allocates a new context and stores the result in ctx
 */
int dt_init_context(dt_context **ctx, xcb_connection_t *dpy, xcb_window_t win);
/**
 * Frees an allocated context
 */
void dt_free_context(dt_context *ctx);

int dt_load_font(xcb_connection_t *dis, dt_font **res, char const *name, int size);
int dt_load_fonts(xcb_connection_t *dis, dt_font **res, char const *name, int n, int size);
void dt_free_font(xcb_connection_t *dis, dt_font *fnt);

/**
 * draws the string composed of the first `len` characters of `txt`,
 * drawn with font `fnt`, in color `color`, with the baseline starting at position
 * x, y.
 */
int dt_draw(dt_context *ctx, dt_font *fnt, dt_color const *color,
                 uint32_t x, uint32_t y, char const *txt, size_t len);

/**
 * Returns the width in pixels that the specified text will take when drawn
 */
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

/**
 * Calls dt_draw for each line such that each line is drawn right after each other
 */
int dt_draw_all_lines(dt_context *ctx, dt_font *fnt, dt_color const *color,
        uint32_t x, uint32_t starting_y, uint32_t padding, char const *lines, int num_lines);
#endif
