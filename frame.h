#ifndef _FRAME_H_
#define _FRAME_H_

#include "canvas.h"

#define BORDER_STYLE_1 ((style_t){ .seq = ESC"[4;48;5;91m" })
#define BORDER_STYLE_2 ((style_t){ .seq = ESC"[5;38;5;16;48;5;73m" })
#define BORDER_STYLE_3 ((style_t){ .seq = ESC"[6;38;5;202;48;5;23m" })
#define BORDER_STYLE_4 ((style_t){ .seq = ESC"[31m" })

size_t register_behavior(canvas_t *canvas);
size_t create_frame(canvas_t *const canvas, vec2_t pos, vec2_t box);
size_t create_styled_frame(canvas_t *const canvas, vec2_t pos, vec2_t box, style_t style);
void destroy_frame(canvas_t *const canvas, const size_t frame_id);

void render_frame(const canvas_t *const canvas, display_t *const display, const size_t frame_id);

#endif// _FRAME_H_
