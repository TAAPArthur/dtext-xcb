#include <string.h>
#include "dtext.h"


int split_lines(char *txt) {
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

int word_wrap_n(xcb_connection_t*dis , dt_font *fnt, char *txt, int num_lines, uint32_t width) {
    char*ptr = txt;
    for(int i=0;i<num_lines;i++) {
        if(get_text_width(dis, fnt, ptr, strlen(ptr)) > width) {
            char* whitespace = ptr;
            char* space = NULL;
            while(whitespace) {
                space = strstr(whitespace + 1, " ");
                if(!space)
                    break;
                int sublen = space - ptr;
                if(get_text_width(dis, fnt, ptr, sublen) > width)
                    break;
                whitespace = space;
            }
            if(whitespace != ptr) {
                *whitespace = 0;
                num_lines++;
            }
            else if(space) {
                *space = 0;
                num_lines++;
            }
        }
        ptr = ptr + strlen(ptr) + 1;
    }
    return num_lines;
}

int word_wrap_line(xcb_connection_t*dis, dt_font *fnt, char *txt, uint32_t width) {
    return word_wrap_n(dis, fnt, txt, split_lines(txt), width);
}

int dt_draw_all_lines(dt_context *ctx, dt_font *fnt, dt_color const *color,
        uint32_t x, uint32_t starting_y, uint32_t padding, char const *lines, int num_lines) {

    int cell_height = padding + get_font_ascent(fnt);
    const char*ptr = lines;
    for(int i=0;i<num_lines;i++) {
        dt_draw(ctx, fnt, color, x, starting_y + cell_height * (i+1), ptr, strlen(ptr));
        ptr = ptr + strlen(ptr) + 1;
    }
    return starting_y + cell_height * num_lines;
}
