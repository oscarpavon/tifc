#include "input.h"

#include <stdio.h>
#include <unistd.h>

static void on_hover(const mouse_event_t *const hover, void *const param)
{
    (void) param;
    printf("hover at %u, %u\n", hover->position.x, hover->position.y);
}

static void on_press(const mouse_event_t *const press, void *const param)
{
    (void) param;
    printf("press %d, at %u, %u\n",
        press->mouse_button,
        press->position.x, press->position.y);
}

static void on_release(const mouse_event_t *const press, void *const param)
{
    (void) param;
    printf("release %d, at %u, %u\n",
        press->mouse_button,
        press->position.x, press->position.y);
}

static void on_drag_begin(const mouse_event_t *const drag_begin,
        void *const param)
{
    (void) param;
    printf("drag %d begin, at %u, %u\n",
        drag_begin->mouse_button,
        drag_begin->position.x, drag_begin->position.y);
}

static void on_drag(const mouse_event_t *const begin,
        const mouse_event_t *const moved, void *const param)
{
    (void) param;
    printf("drag %d at %u, %u\n",
        begin->mouse_button,
        moved->position.x, moved->position.y);
}

static void on_drag_end(const mouse_event_t *const drag_begin,
        const mouse_event_t *const drag_end, void *const param)
{
    (void) param;
    printf("drag end %d from %u, %u to %u, %u\n",
        drag_begin->mouse_button,
        drag_begin->position.x, drag_begin->position.y,
        drag_end->position.x, drag_end->position.y);
}

static void on_scroll(const mouse_event_t *const scroll, void *const param)
{
    (void) param;
    printf("scroll %d at %u, %u\n",
        scroll->mouse_button,
        scroll->position.x, scroll->position.y);
}

int main(void)
{
    input_hooks_t hooks = {
        .on_hover = on_hover,
        .on_press = on_press,
        .on_release = on_release,
        .on_drag_begin = on_drag_begin,
        .on_drag = on_drag,
        .on_drag_end = on_drag_end,
        .on_scroll = on_scroll,
    };
    input_t input = {0};
    input_enable_mouse();

    while (1)
    {
        input_read(&input, &hooks, NULL);
        usleep(100);
    }

    input_disable_mouse();
}
