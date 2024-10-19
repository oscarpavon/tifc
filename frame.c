#include "frame.h"
#include "canvas.h"
#include "sparse.h"
#include "vector.h"

/// !!! not thread safe !!!
///
/// Initializes behavior if yet not initialized,
/// otherwise just returns a cached behavior id.
size_t init_frame_behavior(canvas_t *canvas)
{
    static bool initialized = false;
    static size_t frame_behavior_id;
    if (initialized) return frame_behavior_id;

    frame_behavior_id = canvas_create_behavior(canvas,
        (behavior_opts_t){
            .render = render_frame,
        }
    );
    initialized = true;

    return frame_behavior_id;
}

size_t create_frame(canvas_t *const canvas, vec2_t pos, vec2_t box)
{
    components_t *const components = &canvas->components;
    size_t frame_id = components->last_id++;
    size_t behavior_id = init_frame_behavior(canvas);
    transform_comp_t transform = {.location = pos};
    sparse_insert(&components->transform, frame_id, &transform);
    sparse_insert(&components->box, frame_id, &box);
    sparse_insert(&components->behavior, frame_id, &behavior_id);
    dynarr_append(&components->frames, &frame_id);
    return frame_id;
}

void render_frame(const size_t id, const components_t *const components, display_t *const display)
{
    transform_comp_t *camera = sparse_get(components->transform, components->camera_id);
    vec2_t camera_pos = shifted_camera_position(camera);
    transform_comp_t *frame_transform = sparse_get(components->transform, id);
    vec2_t *frame_box = sparse_get(components->box, id);

    vec2_t beg = {
        .x = frame_transform->location.x + frame_transform->delta.x - camera_pos.x,
        .y = frame_transform->location.y + frame_transform->delta.y - camera_pos.y,
    };
    vec2_t end = {
        beg.x + frame_box->x - 1,
        beg.y + frame_box->y - 1
    };

    for (long long y = beg.y; y <= end.y; ++y)
    {
        for (long long x = beg.x; x <= end.x; ++x)
        {
            // render
            if (x >= 0 && y >= 0 && x < display->size.x && y < display->size.y) // inside screen
            {
                disp_pos_t pos = {x, y};
                display_set_char(display, U' ', pos);
                if (x == beg.x && y == beg.y) display_set_char(display, U'╔', pos);
                else if (x == end.x && y == beg.y) display_set_char(display, U'╗', pos);
                else if (x == end.x && y == end.y) display_set_char(display, U'╝', pos);
                else if (x == beg.x && y == end.y) display_set_char(display, U'╚', pos);
                else if (x == beg.x || x == end.x) display_set_char(display, U'║', pos);
                else if (y == beg.y || y == end.y) display_set_char(display, U'═', pos);
            }
        }
    }
}
