/* See LICENSE file for copyright and license details. */


#include <freetype2/ft2build.h>
#include <xcb/render.h>
#include <xcb/render.h>
#include <xcb/xcb.h>
#include <xcb/xcb_renderutil.h>
#include FT_ADVANCES_H
#include FT_FREETYPE_H

#include "dtext.h"

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

struct dt_context {
    xcb_connection_t* dis;
    xcb_render_pictformat_t win_format;
    xcb_render_picture_t pic;
    xcb_render_picture_t fill;
};

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
struct dt_font {
    uint16_t height;
    uint16_t ascent;

    FT_Library ft_lib;
    FT_Face *faces;
    size_t num_faces;

    xcb_render_glyphset_t gs;
    dt_row advance[DT_HASH_SIZE];
};

uint16_t dt_get_font_ascent(dt_font* font) {
    return font->ascent;
}

uint16_t dt_get_font_height(dt_font* font) {
    return font->height;
}

static xcb_render_pictformat_t get_argb32_format(xcb_connection_t* dis) {
    static xcb_render_pictformat_t argb32_format;
    if (!argb32_format) {
        xcb_render_query_pict_formats_reply_t* reply;
        reply = xcb_render_query_pict_formats_reply(dis, xcb_render_query_pict_formats(dis), NULL);
        xcb_render_pictforminfo_t* pictforminfo = xcb_render_util_find_standard_format(reply, XCB_PICT_STANDARD_ARGB_32);
        argb32_format = pictforminfo ? pictforminfo->id: 0 ;
        free(reply);
    }
    return argb32_format;
}

static dt_pair const * hash_get(dt_row map[DT_HASH_SIZE], uint8_t key) {
    dt_row row;
    size_t i;

    row = map[key % DT_HASH_SIZE];
    for (i = 0; i < row.len; ++i)
        if (row.data[i].c == key)
            return &row.data[i];

    return NULL;
}

static int hash_set(dt_row map[DT_HASH_SIZE], dt_pair val) {
    dt_row row;
    dt_pair *d;
    size_t i;

    row = map[val.c % DT_HASH_SIZE];

    for (i = 0; i < row.len; ++i) {
        if (row.data[i].c == val.c) {
            row.data[i] = val;
            return 0;
        }
    }

    if (row.allocated == row.len) {
        d = row.data;
        if (!(d = realloc(d, (2 * row.len + 1) * sizeof(d[0]))))
            return -1;
        row.data = d;
        row.allocated = 2 * row.len + 1;
    }
    ++row.len;

    row.data[row.len - 1] = val;

    map[val.c % DT_HASH_SIZE] = row;
    return 0;
}

static int load_char(xcb_connection_t* dis, dt_font *fnt, char c) {
    int err;
    FT_UInt code;
    FT_GlyphSlot slot;
    xcb_render_glyph_t gid;
    xcb_render_glyphinfo_t g;
    char *img;
    size_t x, y, i;

    if (hash_get(fnt->advance, c))
        return 0;

    slot = 0;
    for (i = 0; i < fnt->num_faces; ++i) {
        code = FT_Get_Char_Index(fnt->faces[i], c);
        if (!code)
            continue;

        if ((err = FT_Load_Glyph(fnt->faces[i], code, FT_LOAD_RENDER)))
            continue;
        slot = fnt->faces[i]->glyph;
        break;
    }
    if (!slot) {
        if ((err = FT_Load_Char(fnt->faces[0], c, FT_LOAD_RENDER)))
            return err;
        slot = fnt->faces[0]->glyph;
    }

    gid = c;

    g.width  = slot->bitmap.width;
    g.height = slot->bitmap.rows;
    g.x = - slot->bitmap_left;
    g.y =   slot->bitmap_top;
    g.x_off = slot->advance.x >> 6;
    g.y_off = slot->advance.y >> 6;

    if (!(img = malloc(4 * g.width * g.height)))
        return -1;
    for (y = 0; y < g.height; ++y)
        for (x = 0; x < g.width; ++x)
            for (i = 0; i < 4; ++i)
                img[4 * (y * g.width + x) + i] =
                    slot->bitmap.buffer[y * g.width + x];

    xcb_render_add_glyphs(dis, fnt->gs, 1, &gid, &g,
                     4 * g.width * g.height, (uint8_t*) img);

    free(img);

    return hash_set(fnt->advance, (dt_pair) {
        .c = c,
        .adv = slot->advance.x >> 6,
        .asc = - slot->metrics.horiBearingY >> 6,
        .h = slot->metrics.height >> 6
    });
}

static int load_face(FT_Library* ft_lib, FT_Face *face, char const *file, int size) {
    int err = FT_New_Face(*ft_lib, file, 0, face);
    if (err)
        return err;

    if ((err = FT_Set_Char_Size(*face, size << 6, 0, 0, 0))) {
        FT_Done_Face(*face);
        return err;
    }

    return 0;
}

int dt_init_context(dt_context **res, xcb_connection_t *dis, xcb_window_t win) {
    dt_context *ctx;
    xcb_pixmap_t pix;

    if (!(ctx = malloc(sizeof(*ctx))))
        return -1;

    xcb_screen_iterator_t iter = xcb_setup_roots_iterator (xcb_get_setup (dis));
    xcb_screen_t* screen = iter.data;

    ctx->dis = dis;
    ctx->pic = xcb_generate_id(dis);

    xcb_render_query_pict_formats_reply_t* reply;
    reply = xcb_render_query_pict_formats_reply(dis, xcb_render_query_pict_formats(dis), NULL);
    xcb_render_pictvisual_t*  pictvisual;
    pictvisual = xcb_render_util_find_visual_format(reply, screen->root_visual);
    ctx->win_format = pictvisual ? pictvisual->format: 0;
    free(reply);
    xcb_render_create_picture(dis, ctx->pic, win, ctx->win_format, 0, NULL);

    pix = xcb_generate_id(dis);
    xcb_create_pixmap(dis, 32, pix , screen->root, 1, 1);
    uint32_t values = XCB_RENDER_REPEAT_NORMAL;

    ctx->fill = xcb_generate_id(dis);
    xcb_render_create_picture(dis, ctx->fill, pix, get_argb32_format(dis), XCB_RENDER_CP_REPEAT, &values);
    xcb_free_pixmap(dis, pix);

    *res = ctx;
    return 0;
}

void dt_free_context(dt_context *ctx) {
    xcb_render_free_picture(ctx->dis, ctx->pic);
    xcb_render_free_picture(ctx->dis, ctx->fill);
    free(ctx);
}

int dt_load_fonts(xcb_connection_t *dis, dt_font **res, char const *name, int n, int size) {
    dt_error err;
    dt_font *fnt;
    size_t i;
    int16_t descent;

    if (!(fnt = malloc(sizeof(*fnt))))
        return -1;

    if ((err = FT_Init_FreeType(&fnt->ft_lib))) {
        free(fnt);
        return err;
    }

    fnt->num_faces = n;

    if (!(fnt->faces = malloc(fnt->num_faces * sizeof(fnt->faces[0])))) {
        free(fnt);
        return -1;
    }

    for (i = 0; i < fnt->num_faces; ++i) {
        if ((err = load_face(&fnt->ft_lib, &fnt->faces[i], name, size))) {
            while (--i != (size_t) -1)
                FT_Done_Face(fnt->faces[i]);
            free(fnt->faces);
            free(fnt);
            return err;
        }
        name += strlen(name) + 1;
    }


    fnt->gs = xcb_generate_id(dis);
    xcb_render_create_glyph_set(dis, fnt->gs, get_argb32_format(dis));
    memset(fnt->advance, 0, sizeof(fnt->advance));

    fnt->ascent = descent = 0;
    for (i = 0; i < fnt->num_faces; ++i) {
        fnt->ascent = max(fnt->ascent, fnt->faces[i]->ascender >> 6);
        descent = min(descent, fnt->faces[i]->descender >> 6);
    }
    fnt->height = fnt->ascent - descent;

    *res = fnt;
    return 0;
}

int dt_load_font(xcb_connection_t *dis, dt_font **res, char const *name, int size) {
    return dt_load_fonts(dis, res, name, 1, size);
}

uint16_t dt_get_text_width(xcb_connection_t* dis, dt_font *fnt, char const *txt, size_t len) {
    uint32_t text_width = 0;
    for (int i = 0; i < len; ++i) {
        if ((load_char(dis, fnt, txt[i])))
            continue;
        text_width += hash_get(fnt->advance, txt[i])->adv;
    }
    return text_width;
}

void dt_free_font(xcb_connection_t *dis, dt_font *fnt) {
    xcb_render_free_glyph_set(dis, fnt->gs);
    for (size_t i = 0; i < fnt->num_faces; ++i)
        FT_Done_Face(fnt->faces[i]);
    FT_Done_FreeType(fnt->ft_lib);
    free(fnt);
}

typedef struct {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
} dt_color;
int dt_draw(dt_context *ctx, dt_font *fnt, uint32_t color,
        uint32_t x, uint32_t y, const char *txt, size_t len) {
    int err;
    xcb_render_color_t col;
    size_t i;
    dt_color* c = &color;

    col.red   = (c->red   << 8) + c->red;
    col.green = (c->green << 8) + c->green;
    col.blue  = (c->blue  << 8) + c->blue;
    col.alpha = (c->alpha << 8) + c->alpha;
    xcb_rectangle_t rect ={0, 0, 1, 1};
    xcb_render_fill_rectangles(ctx->dis, XCB_RENDER_PICT_OP_SRC, ctx->fill, col, 1, &rect);

    for (i = 0; i < len; ++i)
        if ((err = load_char(ctx->dis, fnt, txt[i])))
            return err;

    xcb_render_util_composite_text_stream_t* stream = xcb_render_util_composite_text_stream(fnt->gs, len, 0);
    xcb_render_util_glyphs_8(stream, x, y, len, (uint8_t*) txt);
    xcb_render_util_composite_text(ctx->dis, XCB_RENDER_PICT_OP_OVER, ctx->fill, ctx->pic, get_argb32_format(ctx->dis), 0, 0, stream);
    xcb_render_util_composite_text_free(stream);

    return 0;
}
