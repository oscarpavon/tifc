#include "grid.h"
#include "display_types.h"
#include "dynarr.h"
#include "layout.h"

#include <assert.h>


void grid_init(grid_t *const grid,
    uint8_t columns,
    uint8_t rows,
    grid_layout_t column_layout[columns],
    grid_layout_t row_layout[rows])
{
    assert(grid);
    assert(columns > 0);
    assert(rows > 0);
    assert(column_layout);
    assert(row_layout);

    grid->columns = columns;
    grid->rows = rows;

    grid->layout = dynarr_create
        (
            .element_size = sizeof(grid_layout_t),
            .initial_cap = columns + rows,
        );
    
    grid->spans = dynarr_create
        (
            .element_size = sizeof(span_t),
            .initial_cap = columns + rows,
        );
    
    grid->cells = dynarr_create
        (
            .element_size = sizeof(grid_cell_t),
        );

    for (uint8_t c = 0; c < columns; ++c)
    {
        dynarr_append(&grid->layout, &column_layout[c]);
    }

    for (uint8_t r = 0; r < rows; ++r)
    {
        dynarr_append(&grid->layout, &row_layout[r]);
    }
}


void grid_deinit(grid_t *const grid)
{
    dynarr_destroy(grid->layout);
    dynarr_destroy(grid->spans);
    dynarr_destroy(grid->cells);
}


void grid_add_cell(grid_t *const grid, grid_span_t span, text_align_t text_align)
{
    assert(grid);

    assert(span.column.start <= span.column.end);
    assert(span.row.start <= span.row.end);
    assert(span.column.end < grid->columns);
    assert(span.row.end < grid->rows);

    grid_cell_t cell = {
        .text_align = text_align,
        .span = span,
    };
    (void) dynarr_append(&grid->cells, &cell);
}

// static
// void cell_recalculate_layout(grid_cell_t *const cell, disp_area_t *const bounds)
// {
//
// }


void grid_recalculate_layout(grid_t *const grid, const disp_area_t *const panel_area)
{
    assert(grid);
    assert(panel_area);

    uint16_t width = panel_area->second.x - panel_area->first.x - 2;
    uint16_t start = panel_area->first.x + 1; // exclude border
    for (size_t c = 0; c < grid->columns; ++c)
    {
        // access layout
        const grid_layout_t *layout = dynarr_get(grid->layout, c);

        span_t *span = dynarr_get(grid->spans, c);
        span->start = start;

        uint16_t size = (LAYOUT_SIZE_FIXED == layout->size_method)
            ? layout->size
            : layout->size * width / 100;

        span->end = span->start + size - 1;
        width -= size;
        start += size;
    }

    uint16_t height = panel_area->second.y - panel_area->first.y - 2;
    start = panel_area->first.y + 1;
    for (size_t r = 0; r < grid->rows; ++r)
    {
        // access layout
        const grid_layout_t *layout = dynarr_get(grid->layout, r + grid->columns);

        span_t *span = dynarr_get(grid->spans, r + grid->columns);
        span->start = start;

        uint16_t size = (LAYOUT_SIZE_FIXED == layout->size_method)
            ? layout->size
            : layout->size * height / 100;

        span->end = span->start + size - 1;
        height -= size;
        start += size;
    }

    const size_t cells_amount = dynarr_size(grid->cells);
    for (size_t i = 0; i < cells_amount; ++i)
    {
        grid_cell_t *cell = dynarr_get(grid->cells, i);

        uint16_t column = cell->span.column.start;
        uint16_t row = cell->span.row.start;

        span_t *first_column = dynarr_get(grid->spans, column);
        span_t *first_row = dynarr_get(grid->spans, grid->columns + row);

        column = cell->span.column.end;
        row = cell->span.row.end;

        span_t *last_column = dynarr_get(grid->spans, column);
        span_t *last_row = dynarr_get(grid->spans, grid->columns + row);

        disp_area_t area = {
            .first = {
                .x = first_column->start,
                .y = first_row->start,
            },
            .second = {
                .x = last_column->end,
                .y = last_row->end,
            },
        };

        cell->area = area;
    }
}
