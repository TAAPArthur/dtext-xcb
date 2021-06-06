#include <string.h>
#include "dtext.h"

int word_wrap(char *txt) {
    char* ptr = txt;
    int num_lines;
    for(num_lines = 0; ptr; num_lines++) {
        ptr = strstr(ptr, "\n");
        if(ptr) {
            *ptr = 0;
            ptr++;
        }
    }
    return num_lines;
}

void dt_draw_all_lines(dt_context *ctx, dt_font *fnt, dt_color const *color,
        uint32_t x, uint32_t starting_y, uint32_t padding, char const *lines, int num_lines) {

    const char*ptr = lines;
    for(int i=0;i<num_lines;i++) {
        dt_draw(ctx, fnt, color, x, starting_y + padding*(i+1) + get_font_ascent(fnt) * i, ptr, strlen(ptr));
        ptr = ptr + strlen(lines) + 1;
    }
}
