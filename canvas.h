#ifndef _CANVAS_H_
#define _CANVAS_H_

#include "input.h"
#include "display.h"

#include "dynarr.h"

typedef struct vec2
{
    long long x;
    long long y;
}
vec2_t;

typedef struct camera
{
    vec2_t position; /* position on infinite canvas */
    vec2_t shift;
}
camera_t;

typedef struct canvas
{
    camera_t camera;
    dynarr_t *objects;
    input_hooks_t input_hooks;
}
canvas_t;

typedef struct object
{
    vec2_t location; // top left corner
    vec2_t box; // width and height
    vec2_t shift; // position delta on dragging
    size_t parent; // index of the parent object
    size_t behavior; // index into a table of functions
}
object_t;

canvas_t canvas_init(void);
void canvas_deinit(canvas_t *const canvas);
void pick_objects(disp_pos_t position, dynarr_t *picked);


void canvas_update(canvas_t *const canvas);
void canvas_render(const canvas_t *const canvas, display_t *const display);

#endif//_CANVAS_H_
