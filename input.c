#include "input.h"

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

static void handle_mouse(input_t *const input, const input_hooks_t *const hooks, void *const param);
static mouse_event_t decode_mouse_event(unsigned char *buffer);
static void print_mouse_event(const mouse_event_t *const event);


void input_enable_mouse(void)
{
    // disable canon mode and echo 
    struct termios attr;
    tcgetattr(fileno(stdin), &attr);
    attr.c_lflag ^= ICANON | ECHO;
    tcsetattr(fileno(stdin), TCSANOW, &attr);
    fprintf(stderr,
        MOUSE_EVENTS_ON
        PASTE_MODE_ON);
}


void input_disable_mouse(void)
{
    // enable canon mode and echo 
    struct termios attr;
    tcgetattr(fileno(stdin), &attr);
    attr.c_lflag ^= ICANON | ECHO;
    tcsetattr(fileno(stdin), TCSANOW, &attr);
    fprintf(stderr,
        MOUSE_EVENTS_OFF
        PASTE_MODE_OFF);
}


void input_read(input_t *input, const input_hooks_t *const hooks, void *const param)
{
    memset(input->input_buffer, 0, INPUT_BUFFER_SIZE);

    input->input_bytes = read(fileno(stdin), input->input_buffer, INPUT_BUFFER_SIZE);
    if (0 == memcmp(input->input_buffer, MOUSE_EVENT_HEADER, sizeof(MOUSE_EVENT_HEADER) - 1))
    {
        handle_mouse(input, hooks, param);
    }
    else {
        printf("%x %x %x %x %x %x\n", 
            input->input_buffer[0],
            input->input_buffer[1],
            input->input_buffer[2],
            input->input_buffer[3],
            input->input_buffer[4],
            input->input_buffer[5]);
    }
}


static void handle_mouse(input_t *const input, const input_hooks_t *const hooks, void *const param)
{
    mouse_event_t event = decode_mouse_event(input->input_buffer);
    print_mouse_event(&event);

    input->prev_mouse_event = input->last_mouse_event;
    input->last_mouse_event = event;

    const mouse_event_t *const prev = &input->prev_mouse_event;
    const mouse_event_t *const last = &input->last_mouse_event;

    if ( (MOUSE_STATIC == prev->motion || MOUSE_MOVING == prev->motion)
        && MOUSE_NONE == prev->mouse_button
        && MOUSE_NONE != last->mouse_button)
    {
        input->mouse_pressed = *last;
        hooks->on_press(&input->mouse_pressed, param);
    }

    if ( MOUSE_STATIC == prev->motion
        && MOUSE_NONE != prev->mouse_button)
    {
        if ( MOUSE_MOVING == last->motion
            && MOUSE_NONE != last->mouse_button)
        {
            input->drag = true;
            hooks->on_drag_begin(&input->mouse_pressed, param);
        }
    }

    if ( (MOUSE_STATIC == prev->motion || MOUSE_MOVING == prev->motion)
        && MOUSE_NONE != prev->mouse_button )
    {
        if ( MOUSE_STATIC == last->motion
            && MOUSE_NONE == last->mouse_button)
        {
            if (!input->drag)
            {
                hooks->on_release(&input->mouse_pressed, param);
            }
            else
            {
                input->drag = false;
                hooks->on_drag_end(&input->mouse_pressed, &input->mouse_released, param);
            }
            input->mouse_released = *last;
        }
    }

    if (MOUSE_SCROLLING == last->motion)
    {
        hooks->on_scroll(last, param);
    }
}


static mouse_event_t decode_mouse_event(unsigned char *buffer)
{
    mouse_event_t event = {
        .mouse_button = buffer[3] & 0x3    /*2 bits*/,
        .modifier = (buffer[3] >> 2) & 0x7 /*3 bits*/,
        .motion = (buffer[3] >> 5) & 0x3   /*2 bits*/,
        .position = {
            buffer[4] - MOUSE_OFFSET,
            buffer[5] - MOUSE_OFFSET
        },
    };

    return event;
}


static void print_mouse_event(const mouse_event_t *const event)
{
    fprintf(stderr, "MOUSE_EVENT (button: %u"
        ", mod:[shift: %u, alt: %u, ctrl: %u]"
        ", motion: %s, x: %d, y: %d)\n",
        event->mouse_button,
        event->modifier & 0x1, (event->modifier >> 1) & 0x1, (event->modifier >> 2) & 0x1,
        event->motion == MOUSE_STATIC ? "static" :
        event->motion == MOUSE_MOVING ? "moving" : "scroll",
        event->position.x, event->position.y);
}
