#include "panel.h"
#include "display.h"
#include "layout.h"

#include <assert.h>

static void
centralize_vertical(unsigned int vertical_size,
                    unsigned int vmax,
                    screen_space_bounds_t *panel_area,
                    screen_space_bounds_t *bounds)
{
    panel_area->first.y = bounds->first.y;
    panel_area->second.y = bounds->second.y;
    if (vmax && vmax < vertical_size)
    {
        unsigned int padding = vertical_size - vmax;
        unsigned int top = padding / 2;
        unsigned int bot = padding - top;
        panel_area->first.y += top;
        panel_area->second.y -= bot;
    }
}

static void
centralize_horizontal(unsigned int horizontal_size,
                      unsigned int hmax,
                      screen_space_bounds_t *panel_area,
                      screen_space_bounds_t *bounds)
{
        panel_area->first.x = bounds->first.x;
        panel_area->second.x = bounds->second.x;
        if (hmax && hmax < horizontal_size)
        {
            unsigned int padding = horizontal_size - hmax;
            unsigned int left = padding / 2;
            unsigned int right = padding - left;
            panel_area->first.x += left;
            panel_area->second.x -= right;
        }
}

static void
dock_to_top(unsigned int vertical_size,
            unsigned int vmax,
            screen_space_bounds_t *panel_area,
            screen_space_bounds_t *bounds)
{
    panel_area->first.y = bounds->first.y;
    vmax = (vertical_size > vmax) ? vmax : vertical_size;
    panel_area->second.y = panel_area->first.y + vmax - 1;
    bounds->first.y = panel_area->second.y + 1;
}

static void
dock_to_bot(unsigned int vertical_size,
            unsigned int vmax,
            screen_space_bounds_t *panel_area,
            screen_space_bounds_t *bounds)
{
    panel_area->second.y = bounds->second.y;
    vmax = (vertical_size > vmax) ? vmax : vertical_size;
    panel_area->first.y = panel_area->second.y - vmax + 1;
    bounds->second.y = panel_area->first.y - 1;
}

static void
dock_to_left(unsigned int horizontal_size,
             unsigned int hmax,
             screen_space_bounds_t *panel_area,
             screen_space_bounds_t *bounds)
{
    panel_area->first.x = bounds->first.x;
    hmax = (horizontal_size > hmax) ? hmax : horizontal_size;
    panel_area->second.x = panel_area->first.x + hmax - 1;
    bounds->first.x = panel_area->second.x + 1;
}

static void
dock_to_right(unsigned int horizontal_size,
              unsigned int hmax,
              screen_space_bounds_t *panel_area,
              screen_space_bounds_t *bounds)
{
    panel_area->second.x = bounds->second.x;
    hmax = (horizontal_size > hmax) ? hmax : horizontal_size;
    panel_area->first.x = panel_area->second.x - hmax + 1;
    bounds->second.x = panel_area->first.x - 1;
}

static void
fill_vertical(screen_space_bounds_t *panel_area,
              screen_space_bounds_t *bounds)
{
    panel_area->first.y = bounds->first.y;
    panel_area->second.y = bounds->second.y;
}

static void
fill_horizontal(screen_space_bounds_t *panel_area,
                screen_space_bounds_t *bounds)
{
    panel_area->first.x = bounds->first.x;
    panel_area->second.x = bounds->second.x;
}

void panel_render(const panel_t *panel,
                  display_t *const display,
                  screen_space_bounds_t *const bounds)
{
    assert(panel);
    const layout_t *const layout = &panel->layout;
    const disp_pos_t *const max = &layout->max;
    // const disp_pos_t *const min = &layout->min;
    unsigned int horizontal_size = bounds->second.x - bounds->first.x;
    unsigned int vertical_size = bounds->second.y - bounds->first.y;

    if (horizontal_size == 0 || vertical_size == 0) return; // TODO: how to handle this?

    screen_space_bounds_t panel_area = {0};

    if (LAYOUT_ALIGN_CENTER == layout->align
        || 0 == layout->align)
    {
        centralize_vertical(vertical_size, max->y, &panel_area, bounds);
        centralize_horizontal(horizontal_size, max->x, &panel_area, bounds);
        *bounds = (screen_space_bounds_t){0}; /* no free space left */
    }
    else if (LAYOUT_ALIGN_TOP_H_CENTER == layout->align)
    {
        centralize_horizontal(horizontal_size, max->x, &panel_area, bounds);
        dock_to_top(vertical_size, max->y, &panel_area, bounds);
    }
    else if (LAYOUT_ALIGN_BOT_H_CENTER == layout->align)
    {
        centralize_horizontal(horizontal_size, max->x, &panel_area, bounds);
        dock_to_bot(vertical_size, max->y, &panel_area, bounds);
    }
    else if (LAYOUT_ALIGN_LEFT_V_CENTER == layout->align)
    {
        centralize_vertical(vertical_size, max->y, &panel_area, bounds);
        dock_to_left(horizontal_size, max->x, &panel_area, bounds);
    }
    else if (LAYOUT_ALIGN_RIGHT_V_CENTER == layout->align)
    {
        centralize_vertical(vertical_size, max->y, &panel_area, bounds);
        dock_to_right(horizontal_size, max->x, &panel_area, bounds);
    }
    else if ((LAYOUT_ALIGN_TOP | LAYOUT_ALIGN_BOT) & layout->align)
    {
        fill_horizontal(&panel_area, bounds);
        (LAYOUT_ALIGN_TOP & layout->align)
            ? dock_to_top(vertical_size, max->y, &panel_area, bounds)
            : dock_to_bot(vertical_size, max->y, &panel_area, bounds);
    }
    else if ((LAYOUT_ALIGN_LEFT | LAYOUT_ALIGN_RIGHT) & layout->align)
    {
        fill_vertical(&panel_area, bounds);
        (LAYOUT_ALIGN_LEFT & layout->align)
            ? dock_to_left(horizontal_size, max->x, &panel_area, bounds)
            : dock_to_right(horizontal_size, max->x, &panel_area, bounds);
    }

    // border_set_t border = {._ = L"╔╗╝╚║═"};
    // TODO: RENDER TO DISPLAY
    // display_draw_border(display, panel->style, border, panel_area);
    display_fill_area(display, panel->style, panel_area);
}

