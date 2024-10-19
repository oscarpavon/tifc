#ifndef _CANVAS_H_
#define _CANVAS_H_

#include "input.h"
#include "display.h"
#include "sparse.h"

typedef struct vec2
{
    long long x;
    long long y;
}
vec2_t;

typedef struct camera
{
    vec2_t location; /* top left corner of the view on infinite canvas */
    vec2_t delta;
}
camera_t;

typedef struct transform_comp
{
    vec2_t location;
    vec2_t delta;
}
transform_comp_t;


typedef struct components
{
    size_t last_id;
    sparse_t *transform; // transform_comp_t
    sparse_t *box;       // vec2_t - width and heigh
    sparse_t *behavior;  // size_t
}
components_t;

typedef struct behavior
{
    void (*on_hover)(const mouse_event_t *const, void *const);
    void (*on_press)(const mouse_event_t *const, void *const);
    void (*on_release)(const mouse_event_t *const, void *const);
    void (*on_drag_begin)(const mouse_event_t *const, void *const);
    void (*on_drag)(const mouse_event_t *const, const mouse_event_t *const, void *const);
    void (*on_drag_end)(const mouse_event_t *const, const mouse_event_t *const, void *const);
    void (*on_scroll)(const mouse_event_t *const, void *const);

    void (*render)(const size_t id, const components_t *const components, display_t *const display);
}
behavior_t;

typedef struct canvas
{
    size_t camera_id;
    components_t components;
    input_hooks_t input_hooks;
}
canvas_t;


canvas_t canvas_init(void);
void canvas_deinit(canvas_t *const canvas);
void canvas_load_objects(canvas_t *const canvas);
void canvas_render(const canvas_t *const canvas, display_t *const display);

#endif//_CANVAS_H_
