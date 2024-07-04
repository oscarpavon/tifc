#ifndef _INPUT_H_
#define _INPUT_H_

#include "display.h"

#include <stddef.h>

#define INPUT_BUFFER_SIZE 6
#define ESC "\x1b"
#define MOUSE_EVENT_HEADER  ESC "[M"

#define MOUSE_EVENTS_ON     ESC "[?1003h"
#define MOUSE_EVENTS_OFF    ESC "[?1003l"

#define PASTE_MODE_ON       ESC "[?2004h"
#define PASTE_MODE_OFF      ESC "[?2004l"

#define MOUSE_OFFSET 0x20

typedef enum mouse_button
{
    MOUSE_1,
    MOUSE_2,
    MOUSE_3,
    MOUSE_NONE
}
mouse_button_t;

typedef enum mouse_motion
{
    MOUSE_STATIC = 1,
    MOUSE_MOVING,
    MOUSE_SCROLLING
}
mouse_motion_t;

typedef enum input_modifier
{
    MOD_NONE = 0,
    MOD_SHIFT = 1,
    MOD_CTRL = 2
}
input_modifier_t;

typedef struct mouse_event
{
    int mouse_button;
    input_modifier_t modifier;
    mouse_motion_t motion;
    disp_pos_t position;
}
mouse_event_t;

typedef struct input
{
    unsigned char input_buffer[INPUT_BUFFER_SIZE];
    size_t input_bytes;
    mouse_event_t prev_mouse_event;
    mouse_event_t last_mouse_event;
    mouse_event_t mouse_pressed;
    mouse_event_t mouse_released;
    bool drag;
}
input_t;

typedef struct input_hooks
{
    void (*on_press)(const mouse_event_t *const press, void *const param);
    void (*on_release)(const mouse_event_t *const press, void *const param);
    void (*on_drag_begin)(const mouse_event_t *const begin, void *const param);
    void (*on_drag_end)(const mouse_event_t *const begin,
        const mouse_event_t *const end, void *const param);
    void (*on_scroll)(const mouse_event_t *const scroll, void *const param);
}
input_hooks_t;

void input_enable_mouse(void);
void input_disable_mouse(void);
void input_read(input_t *const input, const input_hooks_t *const hooks, void *const param);

#endif//_INPUT_H_
