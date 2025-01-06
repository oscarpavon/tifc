#ifndef _GRID_H_
#define _GRID_H_

#include <stddef.h>
#include <sparse.h>

#include "display.h"
#include "layout.h"

#define MAX_COLUMNS 256

typedef struct
{
    unsigned int columns;
    unsigned int rows;
    sparse_t *cells;
}
grid_t;

typedef struct
{
    unsigned int start;
    unsigned int end;
}
span_t;

typedef struct
{
    span_t column;
    span_t row;
}
grid_span_t;

typedef struct
{
    layout_t layout;
    grid_span_t span;
    disp_area_t area;
}
grid_cell_t;

#endif// _GRID_H_
