#ifndef _PANEL_H_
#define _PANEL_H_

#include "display.h"
#include "layout.h"

typedef struct
{
    const char   *title;
    unsigned int  title_size;
    layout_t      layout;
    style_t       style;
    disp_area_t   area;
}
panel_t;

void panel_render(const panel_t *panel,
                  display_t *const display);

void panel_recalculate_layout(panel_t *panel,
                              disp_area_t *const bounds);
#endif // _PANEL_H_
