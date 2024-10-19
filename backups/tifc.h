#ifndef _TIFC_H_
#define _TIFC_H_

#include <stddef.h>


typedef struct tifc_state tifc_state_t;

typedef enum mouse_event_type
{
    MOUSE_1_PRESSED = 0x0,
    MOUSE_2_PRESSED = 0x1,
    MOUSE_3_PRESSED = 0x2,
    MOUSE_RELEASED  = 0x3,
    MOUSE_STATIC = 0x20,
    MOUSE_MOVED = 0x40,
    MOUSE_SCROLL_UP = 0x60,
    MOUSE_SCROLL_DOWN = 0x61
}
mouse_event_type_t;

typedef struct disp_pos
{
    unsigned int x;
    unsigned int y;
}
disp_pos_t;

typedef struct vec2
{
    long long x;
    long long y;
}
vec2_t;

disp_pos_t tifc_get_window_size(void);
void tifc_mouse_begin(void);
void tifc_mouse_end(void);

void tifc_update_state(tifc_state_t *state);
void tifc_render_grid(tifc_state_t *state, disp_pos_t grid);
void tifc_init(tifc_state_t *state);
void tifc_delete(tifc_state_t *state);

#endif/*_TIFC_H_*/
