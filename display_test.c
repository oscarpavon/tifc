#include "display.h"
#include <termio.h>
#include <unistd.h>

int main(void)
{
    display_t display;
    display_set_resize_handler(&display);
    while (1)
    {
        display_clear(&display);
        for (unsigned int i = 0; i < display.size.y; ++i)
        {
            for (unsigned int j = 0; j < display.size.x; ++j)
            {
                display_set_char(&display, U'=', (disp_pos_t){j, i});
            }
        }
        usleep(100);
        display_render(&display);
    }
    return 0;
}
