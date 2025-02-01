#ifndef _GRID_H_
#define _GRID_H_

#include "dynarr.h"
#include "layout.h"
#include "logger.h"
#include "display.h"

#include <stdint.h>

#define MAX_COLUMNS 256
#define MAX_ROWS 256

typedef struct
{
    uint16_t columns;
    uint16_t rows;

    /* TODO consider arena to store layouts and spans */
    dynarr_t *layout;

    /* columns & rows spans recalculated on resize */
    dynarr_t *spans;

    /* Configured content areas.
        size will extend to `rows * columns` at max. */
    dynarr_t *areas;
}
grid_t;

typedef struct
{
    layout_size_method_t size_method;
    uint16_t             size;
}
grid_layout_t;

typedef struct
{
    uint16_t start;
    uint16_t end;
}
span_t;

#define IS_INVALID_SPAN(span_ptr) ((span_ptr)->start == (uint16_t) -1 \
                                  && (span_ptr)->end == (uint16_t) -1)
#define INVALID_SPAN ((span_t){-1, -1})

typedef struct
{
    span_t column;
    span_t row;
}
grid_span_t;

typedef enum
{
    TEXT_ALIGN_LEFT = 0,
    TEXT_ALIGN_RIGHT,
    TEXT_ALIGN_CENTER
}
text_align_t;

typedef struct
{
    text_align_t text_align;
    grid_span_t span;
    disp_area_t area;
}
grid_area_t;


void grid_init(grid_t *const grid,
    uint8_t columns,
    uint8_t rows,
    grid_layout_t column_layout[columns],
    grid_layout_t row_layout[rows]);

void grid_deinit(grid_t *const grid);


void grid_add_area(grid_t *const grid,
        grid_span_t span,
        text_align_t text_align);

void grid_render(const grid_t *const grid, display_t *const display);

void grid_recalculate_layout(grid_t *const grid,
        const disp_area_t *const panel_area);

#endif// _GRID_H_
