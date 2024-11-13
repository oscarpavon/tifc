#include "tifc.h"
#include "display.h"
#include "frame.h"
#include "layout.h"
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
    display_set_resize_handler(&tifc.display);
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
    canvas_render(&tifc->canvas, &tifc->display);
    // ...
    
    ui_render(&tifc->ui, &tifc->display);
    display_render(&tifc->display);
}

void tifc_create_ui_layour(tifc_t *const tifc)
{
    layout_t layout = {
        .dir = LAYOUT_DIR_HORIZONTAL,
        .align = LAYOUT_ALIGN_TOP,
        // .min = (disp_pos_t){.y = 0},
        .max = (disp_pos_t){.y = 2},
    };
    ui_add_panel(&tifc->ui, "top", layout, BORDER_STYLE_2);

    layout.align = LAYOUT_ALIGN_LEFT;
    layout.max = (disp_pos_t){.x = 10};
    ui_add_panel(&tifc->ui, "left", layout, BORDER_STYLE_3);

    layout.align = LAYOUT_ALIGN_RIGHT;
    layout.max = (disp_pos_t){.x = 20};
    ui_add_panel(&tifc->ui, "right", layout, BORDER_STYLE_3);

    layout.align = LAYOUT_ALIGN_BOT;
    layout.max = (disp_pos_t){.y = 4};
    ui_add_panel(&tifc->ui, "bot", layout, BORDER_STYLE_2);

    layout.align = LAYOUT_ALIGN_CENTER;
    layout.max = (disp_pos_t){.x = 10, .y = 10};
    ui_add_panel(&tifc->ui, "center", layout, BORDER_STYLE_2);
}

int tifc_event_loop(void)
{
    tifc_t tifc = tifc_init();
    tifc_create_ui_layour(&tifc);

    int exit_status = 0;

    while (1)
    {
        input_display_overlay(&tifc.input, (disp_pos_t){.x = 0, .y = 3});
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

