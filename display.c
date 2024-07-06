#include "display.h"

#include <stdio.h>
#include <sys/ioctl.h>
#include <signal.h>

static int prev_buffer(const int active);
static void display_swap_buffers(display_t *const display);
static disp_pos_t get_terminal_size(void);

static volatile bool g_resize_detected = false;
static void resize_handler(int signo, siginfo_t *info, void *ctx)
{
    (void) signo; (void) info; (void) ctx;
    g_resize_detected = true;
}

void display_set_resize_handler(display_t *const display)
{
    struct sigaction action = {0};
    action.sa_sigaction = resize_handler;
    sigaction(SIGWINCH, &action, NULL);
    display->size = get_terminal_size();
    fprintf(stderr, CLEAR);
}

void display_render(display_t *const display)
{
    disp_pos_t screen = get_terminal_size();
    disp_area_t screen_area = {
        .second = {
            .x = screen.x - 1,
            .y = screen.y - 1
        }
    };

    display_render_area(display, screen_area);
}


void display_render_area(display_t *const display, disp_area_t area)
{
    dispbuf_ptr_t active = display->buffers[display->active];
    dispbuf_ptr_t previous = display->buffers[prev_buffer(display->active)];

    bool force_reprint = false;
    if (g_resize_detected)
    {
        display->size = get_terminal_size();
        force_reprint = true;
        g_resize_detected = false;
    }

    for (unsigned int line = area.first.y; line <= area.second.y && line < display->size.y; ++line)
    {
        for (unsigned int col = area.first.x; col <= area.second.x && col < display->size.x; ++col)
        {
            if (force_reprint || active[line][col] != previous[line][col])
            {
                fprintf(stderr, ESC "[%d;%dH%lc", line + 1, col + 1, active[line][col]);
            }
        }
    }

    display_swap_buffers(display);
}


void display_set_char(display_t *const display, wint_t ch, disp_pos_t pos)
{
    display->buffers[display->active][pos.y][pos.x] = ch;
}


void display_clear(display_t *const display)
{
    wmemset(display->buffers[display->active][0], U' ', DISP_MAX_WIDTH * DISP_MAX_HEIGHT);
}


void display_clear_area(display_t *const display, disp_area_t area)
{
    unsigned int length = area.second.x - area.first.x;
    dispbuf_ptr_t dispbuf = display->buffers[display->active];
    for (unsigned int line = area.first.y; line <= area.second.y && line < display->size.y; ++line)
    {
        wchar_t *start = dispbuf[line] + area.first.x;
        wmemset(start, U' ', length);
    }
}


bool disp_pos_equal(disp_pos_t a, disp_pos_t b)
{
    return a.x == b.x && a.y == b.y;
}


disp_area_t normalized_area(disp_area_t area)
{
    disp_pos_t top_left = area.first;
    disp_pos_t bottom_right = area.second;
    if (top_left.x > bottom_right.x)
    {
        unsigned int tmp = top_left.x;
        top_left.x = bottom_right.x;
        bottom_right.x = tmp;
    }
    if (top_left.y > bottom_right.y)
    {
        unsigned int tmp = top_left.y;
        top_left.y = bottom_right.y;
        bottom_right.y = tmp;
    }

    return (disp_area_t) {top_left, bottom_right};
}


static int prev_buffer(const int active)
{
    return (active + DISP_BUFFERS - 1) % DISP_BUFFERS;
}


static void display_swap_buffers(display_t *const display)
{
    display->active = (display->active + 1) % DISP_BUFFERS;
}


static disp_pos_t get_terminal_size(void)
{
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    return (disp_pos_t){w.ws_col, w.ws_row};
}


