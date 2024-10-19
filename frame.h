#ifndef _FRAME_H_
#define _FRAME_H_

#include "canvas.h"

size_t register_behavior(canvas_t *canvas);
size_t create_frame(canvas_t *const canvas, vec2_t pos, vec2_t box);

void render_frame(const size_t id, const components_t *const components, display_t *const display);

#endif// _FRAME_H_
