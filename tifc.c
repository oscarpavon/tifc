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
    tifc_t tifc = {
        .canvas = canvas_init(),
        .input = input_init(),
    };
    setlocale(LC_ALL, "");
    canvas_load_objects(&tifc.canvas);
    input_enable_mouse();
    display_set_resize_handler(&tifc.display);
    return tifc;
}

void tifc_deinit(tifc_t *const tifc)
{
    input_disable_mouse();
    canvas_deinit(&tifc->canvas);
}

void tifc_render(tifc_t *const tifc)
{
    display_clear(&tifc->display);
    canvas_render(&tifc->canvas, &tifc->display);
    // ...
    
    display_render(&tifc->display);
}

void tifc_event_loop(void)
{
    tifc_t tifc = tifc_init();

    while (1)
    {
        input_read(&tifc.input, &tifc.canvas.input_hooks, &tifc.canvas);
        input_display_overlay(&tifc.input, &tifc.display, (disp_pos_t){.x = 0, .y = 10});
        tifc_render(&tifc);
    }

    tifc_deinit(&tifc);
}

int main(void)
{
    tifc_event_loop();
    return 0;
}

