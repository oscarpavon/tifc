#include "canvas.h"
#include "display.h"
#include "dynarr.h"

#include <endian.h>
#include <stdio.h>

//
// Mouse events
//
static void on_hover(const mouse_event_t *const, void *const);
static void on_press(const mouse_event_t *const, void *const);
static void on_release(const mouse_event_t *const, void *const);
static void on_drag_begin(const mouse_event_t *const, void *const);
static void on_drag(const mouse_event_t *const, const mouse_event_t *const, void *const);
static void on_drag_end(const mouse_event_t *const, const mouse_event_t *const, void *const);
static void on_scroll(const mouse_event_t *const, void *const);

//
// Render routines
//
static void render_grid(const camera_t *const camera,
        display_t *const display,
        const disp_pos_t grid);

static vec2_t shifted_camera_position(const camera_t *const camera);

canvas_t canvas_init(void)
{
    return (canvas_t) {
        .objects = dynarr_create(.element_size = sizeof(object_t)),
        .input_hooks = {
            .on_hover = on_hover,
            .on_press = on_press,
            .on_release = on_release,
            .on_drag_begin = on_drag_begin,
            .on_drag = on_drag,
            .on_drag_end = on_drag_end,
            .on_scroll = on_scroll
        }
    };
}

void canvas_deinit(canvas_t *const canvas)
{
    dynarr_destroy(canvas->objects);
}


void canvas_render(const canvas_t *const canvas, display_t *const display)
{
    render_grid(&canvas->camera, display, (disp_pos_t){30, 15});
}

static void on_hover(const mouse_event_t *const hover, void *const param)
{
    (void) param;
    printf(ROW(1) "hover, at %u, %u\n",
        hover->position.x, hover->position.y);
}

static void on_press(const mouse_event_t *const press, void *const param)
{
    (void) param;
    printf(ROW(1) "press %d, at %u, %u\n",
        press->mouse_button,
        press->position.x, press->position.y);

    //
    // search target object
    // activate act on press
}

static void on_release(const mouse_event_t *const press, void *const param)
{
    (void) param;
    printf(ROW(1) "release %d, at %u, %u\n",
        press->mouse_button,
        press->position.x, press->position.y);
    //
    // same target object
    // act on release
}

static void on_drag_begin(const mouse_event_t *const begin,
        void *const param)
{
    (void) param;
    printf(ROW(1) "drag %d begin, at %u, %u\n",
        begin->mouse_button,
        begin->position.x, begin->position.y);
}

static void on_drag(const mouse_event_t *const begin, const mouse_event_t *const moved, void *const param)
{
    canvas_t *canvas = param;
    printf(ROW(1) "drag %d drag moving to %u, %u\n",
        begin->mouse_button,
        moved->position.x, moved->position.y);

    if (begin->mouse_button == MOUSE_2)
    {
        canvas->camera.shift = (vec2_t){
            .x = (long long)moved->position.x - begin->position.x,
            .y = (long long)moved->position.y - begin->position.y
        };
    }
}

static void on_drag_end(const mouse_event_t *const begin,
        const mouse_event_t *const end, void *const param)
{
    canvas_t *canvas = param;
    printf(ROW(1) "drag %d from %u, %u to %u, %u\n",
        begin->mouse_button,
        begin->position.x, begin->position.y,
        end->position.x, end->position.y);

    if (begin->mouse_button == MOUSE_2)
    {
       canvas->camera.position = (vec2_t){
            canvas->camera.position.x - canvas->camera.shift.x,
            canvas->camera.position.y - canvas->camera.shift.y
        };
        canvas->camera.shift.x = canvas->camera.shift.y = 0;
    }
}

static void on_scroll(const mouse_event_t *const scroll, void *const param)
{
    (void) param;
    printf(ROW(1) "scroll %d at %u, %u\n",
        scroll->mouse_button,
        scroll->position.x, scroll->position.y);
}

static void render_grid(const camera_t *const camera, display_t *const display, const disp_pos_t grid)
{
    vec2_t camera_pos = shifted_camera_position(camera);

    for (unsigned int line = 0; line < display->size.y; ++line)
    {
        for (unsigned int col = 0; col < display->size.x; ++col)
        {
            if (((line + camera_pos.y) % grid.y) == 0
                && ((col + camera_pos.x) % grid.x) == 0)
            {
                display_set_char(display, U'+', (disp_pos_t){col, line});
            }
        }
    }
    printf(ROW(0) "camera_pos:%lld, %lld         \n", camera_pos.x, camera_pos.y);
}



static vec2_t shifted_camera_position(const camera_t *const camera)
{
    return (vec2_t) {
        .x = camera->position.x - camera->shift.x,
        .y = camera->position.y - camera->shift.y
    };
}
