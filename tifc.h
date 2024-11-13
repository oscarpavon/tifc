#ifndef _TIFC_H_
#define _TIFC_H_

#include "display.h"
#include "input.h"
#include "ui.h"
#include "canvas.h"

typedef enum
{
    TIFC_CANVAS_MODE = 0, /* events will propagate to Canvas */
    TIFC_UI_MODE,         /* events will propagate to UI */
}
tifc_mode_t;

typedef struct tifc
{
    display_t    display;
    input_t      input;
    tifc_mode_t  mode;
    ui_t         ui;
    canvas_t     canvas;
}
tifc_t;


void tifc_canvas_mode(tifc_t *const tifc);
void tifc_ui_mode(tifc_t* const tifc);

#endif /* _TIFC_H_ */
