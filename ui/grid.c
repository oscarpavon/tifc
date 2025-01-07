#include "grid.h"
#include "border.h"
#include "display.h"
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

    // TODO: should implement reserve range for dynarr
    dynarr_spread_insert(&grid->spans, 0, columns + rows, TMP_REF(span_t, 0));

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


void grid_render(const grid_t *const grid, display_t *const display)
{
    const size_t cells_amount = dynarr_size(grid->cells);
    for (size_t i = 0; i < cells_amount; ++i)
    {
        grid_cell_t *cell = dynarr_get(grid->cells, i);
        border_set_t border = {._ = L"╭╮╯╰┆┄"};

        if (! IS_INVALID_AREA(&cell->area))
            display_draw_border(display, BORDER_STYLE_1, border, cell->area);
    }
}

static
void calculate_spans(
        size_t start_offset,
        size_t length,
        const size_t spans_amount,
        const size_t offset,
        const dynarr_t *const grid_layout,
        dynarr_t *const grid_spans
    )
{
    bool no_space_left = false;
    size_t size;

    for (size_t i = 0; i < spans_amount; ++i)
    {
        span_t *span = dynarr_get(grid_spans, i + offset);
        if (no_space_left)
        {
            *span = INVALID_SPAN;
            continue;
        }

        // access layout
        const grid_layout_t *layout = dynarr_get(grid_layout, i + offset);

        span->start = start_offset;
        size = (LAYOUT_SIZE_RELATIVE == layout->size_method)
            ? layout->size * length / 100
            : layout->size;

        if (size > length) size = length;

        span->end = span->start + size - 1;
        start_offset += size;
        length -= size;

        if (0 == length) no_space_left = true;
    }
}


void grid_recalculate_layout(grid_t *const grid, const disp_area_t *const panel_area)
{
    assert(grid);
    assert(panel_area);

    const size_t width = panel_area->second.x - panel_area->first.x - 1;
    const size_t height = panel_area->second.y - panel_area->first.y - 1;

    calculate_spans(panel_area->first.x + 1, width, grid->columns,
            0 /* columns offset */,
            grid->layout, grid->spans);

    calculate_spans(panel_area->first.y + 1, height, grid->rows,
            grid->columns /* rows offset */,
            grid->layout, grid->spans);

    const size_t cells_amount = dynarr_size(grid->cells);
    for (size_t i = 0; i < cells_amount; ++i)
    {
        grid_cell_t *cell = dynarr_get(grid->cells, i);

        const size_t column     = cell->span.column.start;
        const size_t column_end = cell->span.column.end;

        const span_t *first_column = dynarr_get(grid->spans, column);
        const span_t *last_column = first_column;

        for (size_t i = column; i < column_end; ++i)
        {
            if (IS_INVALID_SPAN(last_column + 1)) break;
            ++last_column;
        }

        const size_t row        = cell->span.row.start;
        const size_t row_end    = cell->span.row.end;

        const span_t *first_row = dynarr_get(grid->spans, grid->columns + row);
        const span_t *last_row = first_row;

        for (size_t i = row; i < row_end; ++i)
        {
            if (IS_INVALID_SPAN(last_row + 1)) break;
            ++last_row;
        }

        if (IS_INVALID_SPAN(first_column) || IS_INVALID_SPAN(first_row))
        {
            cell->area = INVALID_AREA;
            return; /* exit */
        }

        /* otherwise */
        cell->area = (disp_area_t){
            .first = {
                .x = first_column->start,
                .y = first_row->start,
            },
            .second = {
                .x = last_column->end,
                .y = last_row->end,
            },
        };
    }
}
