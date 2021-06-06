/* See LICENSE file for copyright and license details. */

typedef int32_t dt_error;

typedef struct {
	FT_Library ft_lib;

    xcb_render_pictformat_t win_format;
    xcb_render_pictformat_t argb32_format;

	xcb_connection_t* dis;

	xcb_render_picture_t pic;
	xcb_render_picture_t fill;
} dt_context;

typedef struct {
	uint8_t c;  // char
	uint16_t adv; // advance
	int16_t asc; // ascender
	uint16_t h; // height
} dt_pair;

typedef struct {
	dt_pair *data;
	size_t len;
	size_t allocated;
} dt_row;

#define DT_HASH_SIZE 128
typedef struct {
	uint16_t height;
	uint16_t ascent;

	FT_Face *faces;
	size_t num_faces;

	xcb_render_glyphset_t gs;
	dt_row advance[DT_HASH_SIZE];
} dt_font;

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

dt_error dt_init(dt_context **ctx, xcb_connection_t *dpy, xcb_window_t win);
void dt_quit(dt_context *ctx);

dt_error dt_load(dt_context *ctx, dt_font **fnt, char const *name);
void dt_free(dt_context *ctx, dt_font *fnt);

dt_error dt_box(dt_context *ctx, dt_font *fnt, dt_bbox *bbox,
                char const *txt, size_t len);
dt_error dt_draw(dt_context *ctx, dt_font *fnt, dt_color const *color,
                 uint32_t x, uint32_t y, char const *txt, size_t len);
