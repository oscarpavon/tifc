#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <wchar.h>
#include <string.h>
#include <stdbool.h>

#define DISP_BUFFERS 2
#define DISP_MAX_WIDTH 256
#define DISP_MAX_HEIGHT 256
#define DISP_OVERLAY_SIZE 1024

#define ESC         "\x1b"
#define HOME        ESC "[H"
#define ROW(n) HOME ESC "["#n"B"
#define CLEAR       ESC "[2J"
#define RESET_STYLE ESC "[0m"
#define HIDE_CURSOR ESC "[?25l"
#define SHOW_CURSOR ESC "[?25h"
#define ERASE_LINE  ESC "[K"

typedef struct
{
    unsigned int x;
    unsigned int y;
}
disp_pos_t;

typedef struct
{
    const char *seq;
}
style_t;

typedef struct
{
    style_t style;
    wchar_t ch;
}
disp_char_t;
typedef disp_char_t (*dispbuf_ptr_t)[DISP_MAX_WIDTH];

typedef struct display
{
    disp_char_t buffers[DISP_BUFFERS][DISP_MAX_HEIGHT][DISP_MAX_WIDTH];
    int active; /* index of the active buffer */
    char overlay[DISP_OVERLAY_SIZE];
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
void display_set_style(display_t *const display, style_t style, disp_pos_t pos);
void display_render(display_t *const display);
char* display_overlay(display_t *const display);
void display_render_area(display_t *const display, disp_area_t area);
void display_clear(display_t *const display);
void display_erase(void);
void display_clear_area(display_t *const display, disp_area_t area);
bool disp_pos_equal(disp_pos_t a, disp_pos_t b);
disp_area_t normalized_area(disp_area_t area);

#endif//_DISPLAY_H_
