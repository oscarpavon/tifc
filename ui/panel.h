#ifndef _PANEL_H_
#define _PANEL_H_

#include "display.h"
#include "grid.h"
#include "layout.h"

typedef struct
{
    layout_align_t       align;
    layout_size_method_t size_method;
    disp_pos_t           size;
}
panel_layout_t;

typedef struct
{
    const char    * title;
    unsigned int    title_size;
    panel_layout_t  layout;
    style_t         style;
    disp_area_t     area;
    grid_t          grid;
}
panel_t;

typedef struct
{
    const char     * title;
    panel_layout_t   layout;
}
panel_opts_t;

void panel_create(panel_t *panel,
                  const panel_opts_t *const opts);

void panel_render(const panel_t *panel,
                  display_t *const display);

void panel_recalculate_layout(panel_t *panel,
                              disp_area_t *const bounds);
#endif // _PANEL_H_
