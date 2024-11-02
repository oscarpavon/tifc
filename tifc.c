#include "display.h"
#include "input.h"
#include "canvas.h"

#include <locale.h>

typedef struct tifc
{
    display_t display;
    input_t input;
    canvas_t canvas;
}
tifc_t;

tifc_t tifc_init(void)
{
    setlocale(LC_ALL, "");
    input_enable_mouse();
    tifc_t tifc = {
        .canvas = canvas_init(),
        .input = input_init(),
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

void tifc_render(tifc_t *const tifc)
{
    display_clear(&tifc->display);
    canvas_render(&tifc->canvas, &tifc->display);
    // ...
    
    display_render(&tifc->display);
}

int tifc_event_loop(void)
{
    tifc_t tifc = tifc_init();
    int exit_status = 0;

    while (1)
    {
        input_display_overlay(&tifc.input, &tifc.display, (disp_pos_t){.x = 0, .y = 10});
        tifc_render(&tifc);

        exit_status = input_handle_events(&tifc.input, &tifc.canvas.input_hooks, &tifc.canvas);
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

