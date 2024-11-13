#ifndef _PANEL_H_
#define _PANEL_H_

#include "layout.h"

typedef struct
{
    const char *title;
    layout_t    layout;
    style_t     style;
}
panel_t;

void panel_render(const panel_t *panel,
                  display_t *const display,
                  screen_space_bounds_t *const bounds);

#endif // _PANEL_H_
