#include "tifc.h"
#include "display.h"
#include "frame.h"
#include "grid.h"
#include "layout.h"
#include "panel.h"
#include "ui.h"

#include <locale.h>

tifc_t tifc_init(void)
{
    setlocale(LC_ALL, "");
    input_enable_mouse();
    tifc_t tifc = {
        .input = input_init(),
        .mode = TIFC_CANVAS_MODE,
        .ui = ui_init(),
        .canvas = canvas_init(),
    };
    canvas_load_objects(&tifc.canvas);
    return tifc;
}

void tifc_deinit(tifc_t *const tifc)
{
    input_disable_mouse();
    canvas_deinit(&tifc->canvas);
    input_deinit(&tifc->input);
}

void tifc_canvas_mode(tifc_t *const tifc)
{
    tifc->mode = TIFC_CANVAS_MODE;
}

void tifc_ui_mode(tifc_t* const tifc)
{
    tifc->mode = TIFC_UI_MODE;
}

input_hooks_t* tifc_mode_current_hooks(tifc_t *const tifc)
{
    switch (tifc->mode)
    {
        case TIFC_CANVAS_MODE: return &tifc->canvas.hooks;
        case TIFC_UI_MODE:     return &tifc->ui.hooks;
    }
    return 0;
}

void tifc_render(tifc_t *const tifc)
{
    display_clear(&tifc->display);
    //canvas_render(&tifc->canvas, &tifc->display);
    // ...
    
    ui_render(&tifc->ui, &tifc->display);
    display_render(&tifc->display);
}

void tifc_create_ui_layout(tifc_t *const tifc)
{
    panel_opts_t *opts = &(panel_opts_t)
    {
        .title = "top",
        .layout = {
            .align = LAYOUT_ALIGN_TOP,
            .size_method = LAYOUT_SIZE_RELATIVE,
            .size = {.y = 50},
        },
        .columns = 4,
        .column_layout = (grid_layout_t[]){
            [0] = {.size = 3},
            [1] = {.size = 50, .size_method = LAYOUT_SIZE_RELATIVE },
            [2] = {.size = 50, .size_method = LAYOUT_SIZE_RELATIVE },
            [3] = {.size = 100, .size_method = LAYOUT_SIZE_RELATIVE },
        },
        .rows = 3,
        .row_layout = (grid_layout_t[]){
            [0] = {.size = 3},
            [1] = {.size = 35, .size_method = LAYOUT_SIZE_RELATIVE },
            [2] = {.size = 100, .size_method = LAYOUT_SIZE_RELATIVE },
        }
    };
    panel_t * panel = ui_add_panel(&tifc->ui, opts);
    grid_area_opts_t span;

    span = (grid_area_opts_t){{0, 0}, {0, 0}, TEXT_ALIGN_LEFT};
    grid_add_area(&panel->grid, span);

    span = (grid_area_opts_t){{1, 1}, {0, 0}, TEXT_ALIGN_LEFT};
    grid_add_area(&panel->grid, span);

    span = (grid_area_opts_t){{2, 2}, {0, 0}, TEXT_ALIGN_LEFT};
    grid_add_area(&panel->grid, span);

    span = (grid_area_opts_t){{3, 3}, {0, 0}, TEXT_ALIGN_LEFT};
    grid_add_area(&panel->grid, span);

    span = (grid_area_opts_t){{0, 0}, {1, 1}, TEXT_ALIGN_LEFT};
    grid_add_area(&panel->grid, span);

    span = (grid_area_opts_t){{1, 1}, {1, 1}, TEXT_ALIGN_LEFT};
    grid_add_area(&panel->grid, span);

    span = (grid_area_opts_t){{2, 2}, {1, 1}, TEXT_ALIGN_LEFT};
    grid_add_area(&panel->grid, span);

    span = (grid_area_opts_t){{3, 3}, {1, 1}, TEXT_ALIGN_LEFT};
    grid_add_area(&panel->grid, span);

    span = (grid_area_opts_t){{0, 0}, {2, 2}, TEXT_ALIGN_LEFT};
    grid_add_area(&panel->grid, span);

    span = (grid_area_opts_t){{1, 1}, {2, 2}, TEXT_ALIGN_LEFT};
    grid_add_area(&panel->grid, span);

    span = (grid_area_opts_t){{2, 2}, {2, 2}, TEXT_ALIGN_LEFT};
    grid_add_area(&panel->grid, span);

    span = (grid_area_opts_t){{3, 3}, {2, 2}, TEXT_ALIGN_LEFT};
    grid_add_area(&panel->grid, span);




    opts->title = "left";
    opts->layout.align = LAYOUT_ALIGN_LEFT;
    opts->layout.size.x = 50;
    (void) ui_add_panel(&tifc->ui, opts);

    opts->title = "right-top";
    opts->layout.align = LAYOUT_ALIGN_TOP;
    opts->layout.size.y = 50;
    (void) ui_add_panel(&tifc->ui, opts);

    opts->title = "right-bot";
    opts->layout.align = LAYOUT_ALIGN_BOT;
    opts->layout.size.y = 100;
    (void) ui_add_panel(&tifc->ui, opts);

    ui_recalculate_layout(&tifc->ui, &tifc->display);
}

int tifc_event_loop(void)
{
    tifc_t tifc = tifc_init();
    resize_hook_with_data_t resize_hook = {
        .data = &tifc.ui,
        .hook = ui_resize_hook,
    };
    display_set_resize_handler(&tifc.display, resize_hook);
    tifc_create_ui_layout(&tifc);

    int exit_status = 0;

    while (1)
    {
        // input_display_overlay(&tifc.input, (disp_pos_t){.x = 0, .y = 3});
        tifc_render(&tifc);
        input_hooks_t *hooks = tifc_mode_current_hooks(&tifc);
        exit_status = input_handle_events(&tifc.input, hooks, &tifc);
        if (0 != exit_status)
        {
            display_erase();
            break;
        }
    }

    tifc_deinit(&tifc);
    return exit_status;
}

int main(void)
{
    return tifc_event_loop();
}

