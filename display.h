#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <wchar.h>
#include <stdbool.h>

#define DISP_BUFFERS 2
#define DISP_MAX_WIDTH 256
#define DISP_MAX_HEIGHT 256

typedef wchar_t (*dispbuf_ptr_t)[DISP_MAX_WIDTH];

typedef struct disp_pos
{
    unsigned int x;
    unsigned int y;
}
disp_pos_t;

typedef struct display
{
    wchar_t buffers[DISP_BUFFERS][DISP_MAX_HEIGHT][DISP_MAX_WIDTH];
    int active; /* index of the active buffer */
    disp_pos_t size;
}
display_t;

typedef struct disp_area
{
    disp_pos_t first;
    disp_pos_t second;
}
disp_area_t;

void display_set_resize_handler(display_t *const display);
void display_set_char(display_t *const display, wint_t ch, disp_pos_t pos);
void display_render(display_t *const display);
void display_render_area(display_t *const display, disp_area_t area);
void display_clear(display_t *const display);
void display_clear_area(display_t *const display, disp_area_t area);

bool disp_pos_equal(disp_pos_t a, disp_pos_t b);
disp_area_t normalized_area(disp_area_t area);

#endif//_DISPLAY_H_
