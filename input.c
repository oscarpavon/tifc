#include "input.h"

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <signal.h>
#include <assert.h>
#include <fcntl.h>
#include "display.h"
#include "hash.h"

#define MAX_EVENTS 10

typedef struct
{
    int temp;
}
buffer_t;

// global struct monitoring signals
typedef struct
{
    bool sigint;
}
signal_monitor_t;
static volatile signal_monitor_t s_sm = {0};

static void handle_sigint(int sig);
static void handle_mouse(input_t *const input, const input_hooks_t *const hooks, void *const param);
static mouse_event_t decode_mouse_event(unsigned char *buffer);
static int print_mouse_event(const mouse_event_t *const event, char *overlay, int n);

input_t input_init(void)
{
    int epfd = epoll_create1(0);
    if (epfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    struct epoll_event ev;
    ev.data.fd = STDIN_FILENO; // Monitor standard input
    ev.events = EPOLLIN;
    if (-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev)) {
        perror("epoll_ctl: stdin");
        exit(EXIT_FAILURE);
    }
    hashmap_t *descriptors = hm_create(
        .hashfunc = hash_int,
        .key_size = sizeof(int),
        .value_size = sizeof(buffer_t),
    );

    // Set sigint handler
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sa.sa_flags = 0; // No special flags
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    if (!descriptors)
    {
        exit(EXIT_FAILURE);
    }
    input_t input = {
        .mode = INPUT_MODE_MOUSE,
        .epfd = epfd,
        .descriptors = descriptors,
    };
    return input;
}

void input_deinit(input_t *const input)
{
    hm_destroy(input->descriptors);
    close(input->epfd);
}

void input_enable_mouse(void)
{
    // set unblocking io behavior 
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);


    // disable canon mode and echo 
    struct termios attr;
    tcgetattr(STDIN_FILENO, &attr);
    attr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &attr);
    fprintf(stderr,
        MOUSE_EVENTS_ON
        PASTE_MODE_ON);
}


void input_disable_mouse(void)
{
    // enable canon mode and echo 
    struct termios attr;
    tcgetattr(STDIN_FILENO, &attr);
    attr.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &attr);
    fprintf(stderr,
        MOUSE_EVENTS_OFF
        PASTE_MODE_OFF);
}

int input_handle_events(input_t *const input, const input_hooks_t *const hooks, void *const param)
{
    struct epoll_event events[MAX_EVENTS];
    int events_num = 0;
    // acquire events
    while (true)
    {
        events_num = epoll_wait(input->epfd, events, MAX_EVENTS, -1);
        if (events_num != -1)
        {
            break; // epoll succeded
        }
        else if (errno != EINTR) // something went wrong
        {
            perror("epoll_wait");
            return errno;
        }
        // The call was interrupted by a signal;
        // Process signals and continue:
        if (s_sm.sigint)
        {
            s_sm.sigint = false;
            printf("Exit!\n");
            return -1;
        }
        // Retry epoll_wait
    }

    for (int e = 0; e < events_num; e++)
    {
        if (events[e].events & EPOLLIN)
        {
            // process standard input
            if (events[e].data.fd == STDIN_FILENO)
            {
                // Data available from stdin
                int status = input_read(input, hooks, param);
                if (0 != status)
                {
                    return status;
                }
            }

            { // process buffers
                buffer_t *buffer = hm_get(input->descriptors, &events[e].data.fd);
                if (buffer)
                {
                    // write data to buffer
                }
            }
        }
    }
    return 0;
}

int input_read(input_t *input, const input_hooks_t *const hooks, void *const param)
{
    memset(input->buffer, 0, INPUT_BUFFER_SIZE); // TODO: maybe unnecessary

    int input_bytes = read(STDIN_FILENO, input->buffer, INPUT_BUFFER_SIZE);
    if (input_bytes > 0)
    {
        // Check for Ctrl+D
        if (memchr(input->buffer, '\x04', input_bytes))
        {
            printf("\nEOF detected. Exiting...\n");
            return -1;
            // goto cleanup;
        }
    }
    else if (input_bytes == -1)
    {
        perror("read from stdin");
    }

    if (0 == memcmp(input->buffer, MOUSE_EVENT_HEADER, sizeof(MOUSE_EVENT_HEADER) - 1))
    {
        handle_mouse(input, hooks, param);
    }
    else
    {
        printf("%x %x %x %x %x %x\n",
            input->buffer[0],
            input->buffer[1],
            input->buffer[2],
            input->buffer[3],
            input->buffer[4],
            input->buffer[5]);
    }

    return 0;
}

void input_display_overlay(input_t *const input, display_t *const display, disp_pos_t pos)
{
    char *overlay = display_overlay(display);
    int n = strlen(overlay);
    n += sprintf(overlay + n, ESC "[%d;%dH", pos.y, pos.x);
    n += sprintf(overlay + n, "INPUT MODE: %s", input->mode == INPUT_MODE_TEXT ? "TEXT_MODE" : "MOUSE_MODE");
    if (input->mode == INPUT_MODE_MOUSE)
    {
        n += sprintf(overlay + n, ESC "[%d;%dH", pos.y + 1, pos.x);
        n += print_mouse_event(&input->mouse_mode.last_mouse_event, overlay, n);
    }
    assert(n < DISP_OVERLAY_SIZE);
}

static void handle_mouse(input_t *const input, const input_hooks_t *const hooks, void *const param)
{
    mouse_mode_t *mouse_mode = &input->mouse_mode;
    mouse_event_t event = decode_mouse_event(input->buffer);

    mouse_mode->prev_mouse_event = mouse_mode->last_mouse_event;
    mouse_mode->last_mouse_event = event;

    const mouse_event_t *const prev = &mouse_mode->prev_mouse_event;
    const mouse_event_t *const last = &mouse_mode->last_mouse_event;

    if ( (MOUSE_STATIC == prev->motion || MOUSE_MOVING == prev->motion)
        && MOUSE_NONE == prev->mouse_button
        && MOUSE_NONE != last->mouse_button)
    {
        mouse_mode->mouse_pressed = *last;
        hooks->on_press(&mouse_mode->mouse_pressed, param);
    }

    if ( MOUSE_STATIC == prev->motion
        && MOUSE_NONE != prev->mouse_button)
    {
        if ( MOUSE_MOVING == last->motion
            && MOUSE_NONE != last->mouse_button)
        {
            mouse_mode->drag = true;
            hooks->on_drag_begin(&mouse_mode->mouse_pressed, param);
        }
    }

    if (mouse_mode->drag)
    {
        hooks->on_drag(&mouse_mode->mouse_pressed, &mouse_mode->last_mouse_event, param);
    }
    else if (MOUSE_MOVING == last->motion)
    {
        hooks->on_hover(&mouse_mode->last_mouse_event, param);
    }

    if ( (MOUSE_STATIC == prev->motion || MOUSE_MOVING == prev->motion)
        && MOUSE_NONE != prev->mouse_button )
    {
        if ( MOUSE_STATIC == last->motion
            && MOUSE_NONE == last->mouse_button)
        {
            if (!mouse_mode->drag)
            {
                hooks->on_release(&mouse_mode->mouse_pressed, param);
            }
            else
            {
                mouse_mode->drag = false;
                hooks->on_drag_end(&mouse_mode->mouse_pressed, &mouse_mode->mouse_released, param);
            }
            mouse_mode->mouse_released = *last;
        }
    }

    if (MOUSE_SCROLLING == last->motion)
    {
        hooks->on_scroll(last, param);
    }
}


static mouse_event_t decode_mouse_event(unsigned char *buffer)
{
    mouse_event_t event = {
        .mouse_button = buffer[3] & 0x3    /*2 bits*/,
        .modifier = (buffer[3] >> 2) & 0x7 /*3 bits*/,
        .motion = (buffer[3] >> 5) & 0x3   /*2 bits*/,
        .position = {
            buffer[4] - MOUSE_OFFSET,
            buffer[5] - MOUSE_OFFSET
        },
    };

    return event;
}


static int print_mouse_event(const mouse_event_t *const event, char *overlay, int n)
{
    n = sprintf(overlay + n, "MOUSE_EVENT:\n\tbutton: %u"
        "\n\tmod:[shift: %u, alt: %u, ctrl: %u]"
        "\n\tmotion: %s, x: %d, y: %d)\n",
        event->mouse_button,
        event->modifier & 0x1, (event->modifier >> 1) & 0x1, (event->modifier >> 2) & 0x1,
        event->motion == MOUSE_STATIC ? "static" :
        event->motion == MOUSE_MOVING ? "moving" : "scroll",
        event->position.x, event->position.y);
    return n;
}

// Signal handler for SIGINT (Ctrl+C)
static void handle_sigint(int sig)
{
    s_sm.sigint = true;
} 
