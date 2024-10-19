#include "canvas.h"
#include "display.h"
#include "input.h"

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

static input_hooks_t hooks_init(void)
{
    return (input_hooks_t)
    {
        .on_hover = on_hover,
        .on_press = on_press,
        .on_release = on_release,
        .on_drag_begin = on_drag_begin,
        .on_drag = on_drag,
        .on_drag_end = on_drag_end,
        .on_scroll = on_scroll
    };
}

//
// Components
//
static components_t components_init(void);
static void components_deinit(components_t *const components);


//
// Render routines
//
static void render_grid(transform_comp_t *camera_transform, display_t *const display, const disp_pos_t grid);


static size_t create_camera(components_t *const components);
static vec2_t shifted_camera_position(const transform_comp_t *const camera_transform);


canvas_t canvas_init(void)
{
    return (canvas_t) {
        .components = components_init(),
        .input_hooks = hooks_init()
    };
}

void canvas_deinit(canvas_t *const canvas)
{
    components_deinit(&canvas->components);
}


void canvas_load_objects(canvas_t *const canvas)
{
    canvas->camera_id = create_camera(&canvas->components);
    
}


void canvas_render(const canvas_t *const canvas, display_t *const display)
{

    transform_comp_t *t = sparse_get(canvas->components.transform, canvas->camera_id);
    render_grid(t, display, (disp_pos_t){30, 15});
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
        transform_comp_t *t = sparse_get(canvas->components.transform, canvas->camera_id);

        t->delta = (vec2_t){
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
        transform_comp_t *t = sparse_get(canvas->components.transform, canvas->camera_id);

        t->location = (vec2_t){
            t->location.x - t->delta.x,
            t->location.y - t->delta.y
        };
        t->delta.x = t->delta.y = 0;
    }
}

static void on_scroll(const mouse_event_t *const scroll, void *const param)
{
    (void) param;
    printf(ROW(1) "scroll %d at %u, %u\n",
        scroll->mouse_button,
        scroll->position.x, scroll->position.y);
}


static components_t components_init(void)
{
    return (components_t)
    {
        .transform = sparse_create(.element_size = sizeof(transform_comp_t)),
        .box = sparse_create(.element_size = sizeof(vec2_t)),
        .behavior = sparse_create(.element_size = sizeof(size_t)),
    };
}


static void components_deinit(components_t *const components)
{
    sparse_destroy(components->transform);
    sparse_destroy(components->box);
    sparse_destroy(components->behavior);
}


static void render_grid(transform_comp_t *camera_transform, display_t *const display, const disp_pos_t grid)
{
    vec2_t camera_pos = shifted_camera_position(camera_transform);

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

static size_t create_camera(components_t *const components)
{
    const size_t id = components->last_id++;
    transform_comp_t camera_transform = {0};
    sparse_insert(&components->transform, id, &camera_transform);
    return id;
}

static vec2_t shifted_camera_position(const transform_comp_t *const camera_transform)
{
    return (vec2_t) {
        .x = camera_transform->location.x - camera_transform->delta.x,
        .y = camera_transform->location.y - camera_transform->delta.y
    };
}
