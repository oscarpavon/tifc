#include "tifc.h"
#include "dynarr.h"

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <wchar.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdio.h>
#include <locale.h>

#define DISP_BUFFERS 2
#define DISP_MAX_WIDTH 256
#define DISP_MAX_HEIGHT 256
#define DEFAULT_TEXT_AREA_WIDTH 32ll
#define DEFAULT_TEXT_AREA_HEIGHT 12ll
#define INTERFACES_NUM 10

#define MOUSE_EVENT_HEADER "\x1b[M"
#define MOUSE_EVENTS_ON "\x1b[?1003h"
#define MOUSE_EVENTS_OFF "\x1b[?1003l"
#define MOUSE_POS_OFFSET 0x20
#define MOUSE_BUTTON(type) (type & 0x3)
#define CLEAR_SCREEN "\x1b[2J"
#define HIDE_CURSOR "\x1b[?25l"
#define UNHIDE_CURSOR "\x1b[?25h"
#define HOME "\x1b[H"

#define INPUT_BUFFER_SIZE 6

typedef wchar_t (*dispbuf_ptr_t)[DISP_MAX_WIDTH];

typedef struct display
{
    wchar_t buffers[DISP_BUFFERS][DISP_MAX_HEIGHT][DISP_MAX_WIDTH];
    int active; /* index of the active buffer */
}
display_t;

typedef struct disp_area
{
    disp_pos_t first;
    disp_pos_t second;
}
disp_area_t;

typedef enum tifc_state_type
{
    TIFC_STATE_HOVER = 0,
    TIFC_STATE_CAMERA,
    TIFC_STATE_SELECTION,
    TIFC_STATE_INSERT,
    TIFC_STATE_MOVE
}
tifc_state_type_t;

typedef struct camera
{
    vec2_t position; /* position on infinite canvas */
    vec2_t shift;
}
camera_t;

typedef struct mouse_event
{
    mouse_event_type_t type;
    disp_pos_t position;
}
mouse_event_t;

typedef struct tifc_object
{
    vec2_t location;
    vec2_t box;
    vec2_t shift;
    size_t interface;
    void *data;
}
tifc_object_t;

typedef struct text_area
{
    size_t cursor;
    size_t size;
    wchar_t text[];
}
text_area_t;

typedef struct tifc_interface
{
    void (*mouse_button[MOUSE_RELEASED])(tifc_state_t *state, tifc_object_t *object);
    void (*render)(tifc_state_t *state, tifc_object_t *object);
}
tifc_interface_t;

struct tifc_state
{
    tifc_state_type_t type;
    mouse_event_t mouse_pressed;
    mouse_event_t mouse_released;
    disp_pos_t last_mouse_position;

    camera_t camera;
    disp_area_t selection;
    tifc_object_t *insert_target;
    unsigned char input_buffer[INPUT_BUFFER_SIZE + 1];
    tifc_interface_t interfaces[INTERFACES_NUM];
    dynarr_t *objects;
    display_t display;
};

// Display stuff
static int prev_buffer(const int active);
static void display_swap_buffers(display_t *const display);
static void display_render(display_t *const display);
static void display_set_char(display_t *const display, wint_t ch, disp_pos_t pos);
static void display_clear(display_t *const display);
static disp_area_t normalized_area(disp_area_t area);
static bool disp_pos_equal(disp_pos_t a, disp_pos_t b);

// Input
static void handle_mouse(tifc_state_t *state, unsigned char *input_buffer);
static void handle_keyboard(tifc_state_t *state, unsigned char *input_buffer, const size_t bytes);

// Events
static void handle_camera(tifc_state_t *state);
static void handle_selection(tifc_state_t *state);

// Render
static void render_screen(tifc_state_t *state);
static void render_grid(tifc_state_t *state, disp_pos_t grid);
static void render_selection(tifc_state_t *state);
static void render_objects(tifc_state_t *state);

// Objects
static tifc_object_t *get_target_object(tifc_state_t *state);

// Text area stuff
static text_area_t *create_text_area(size_t size);
static text_area_t *resize_text_area(text_area_t *area, size_t size);

// Text Input Mode
static text_area_t *text_area_insert();
static void handle_text_edit(tifc_state_t *state);

static void text_area_mouse_1(tifc_state_t *state, tifc_object_t *object);
static void text_area_mouse_2(tifc_state_t *state, tifc_object_t *object);
static void text_area_mouse_3(tifc_state_t *state, tifc_object_t *object);
static void text_area_render(tifc_state_t *state, tifc_object_t *object);


disp_pos_t tifc_get_window_size(void)
{
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    // printf ("lines %d\n", w.ws_row);
    // printf ("columns %d\n", w.ws_col);
    return (disp_pos_t){w.ws_col + 1, w.ws_row + 1};
}

void tifc_mouse_begin(void)
{
    // disable canon mode and echo 
    struct termios attr;
    tcgetattr(fileno(stdin), &attr);
    attr.c_lflag ^= ICANON | ECHO;
    tcsetattr(fileno(stdin), TCSANOW, &attr);
    fprintf(stderr, MOUSE_EVENTS_ON);
}

void tifc_mouse_end(void)
{
    // disable canon mode and echo 
    struct termios attr;
    tcgetattr(fileno(stdin), &attr);
    attr.c_lflag ^= ICANON | ECHO;
    tcsetattr(fileno(stdin), TCSANOW, &attr);
    fprintf(stderr, MOUSE_EVENTS_OFF CLEAR_SCREEN);
}

static void render_screen(tifc_state_t *state)
{
    display_clear(&state->display);
    render_grid(state, (disp_pos_t){30, 15});

    render_objects(state);
    render_selection(state);
    display_render(&state->display);
    //FIXME: cursor needs be at the end of the input 
}

static void handle_camera(tifc_state_t *state)
{
    int button_idx = MOUSE_BUTTON(state->mouse_pressed.type);
    if (MOUSE_2_PRESSED != button_idx)
    {
        return;
    }

    state->type = TIFC_STATE_CAMERA;
    state->camera.shift = (vec2_t){
        (long long)state->last_mouse_position.x - state->mouse_pressed.position.x,
        (long long)state->last_mouse_position.y - state->mouse_pressed.position.y
    };

    if (state->mouse_released.type)
    {
        state->camera.position = (vec2_t){
            state->camera.position.x - state->camera.shift.x,
            state->camera.position.y - state->camera.shift.y
        };
        state->camera.shift.x = state->camera.shift.y = 0;
    }
}

static void handle_selection(tifc_state_t *state)
{
    int button_idx = MOUSE_BUTTON(state->mouse_pressed.type);
    if (MOUSE_3_PRESSED != button_idx) return;

    state->type = TIFC_STATE_SELECTION;

    state->selection.first = state->mouse_pressed.position;
    state->selection.second = state->last_mouse_position;
    
    if (state->mouse_released.type)
    {
        
    }
}

static void handle_objects(tifc_state_t *state)
{
    vec2_t mouse_location = {
        (long long)state->camera.position.x + state->mouse_pressed.position.x,
        (long long)state->camera.position.y + state->mouse_pressed.position.y
    };

    size_t size = dynarr_size(state->objects);
    for (size_t i = 0; i < size; ++i)
    {
        tifc_object_t *object = dynarr_get(state->objects, i);

        if (mouse_location.x >= object->location.x
          && mouse_location.x < object->location.x + object->box.x
          && mouse_location.y >= object->location.y
          && mouse_location.y < object->location.y + object->box.y)
        {
            int button_idx = MOUSE_BUTTON(state->mouse_pressed.type);
            state->interfaces[object->interface].mouse_button[button_idx](state, object);
        }
    }
}


static void render_selection(tifc_state_t *state)
{
    disp_area_t area = normalized_area(state->selection);
    disp_pos_t top_right = {area.second.x, area.first.y};
    disp_pos_t bottom_left = {area.first.x,area.second.y};

    if (area.first.x == area.second.x && area.first.y == area.second.y)
    {
        display_set_char(&state->display, U'+', area.first);
        return;
    }

    display_set_char(&state->display, U'⌜', area.first);
    display_set_char(&state->display, U'⌟', area.second);
    display_set_char(&state->display, U'⌝', top_right);
    display_set_char(&state->display, U'⌞', bottom_left);
}

static void render_grid(tifc_state_t *state, disp_pos_t grid)
{
    disp_pos_t screen = tifc_get_window_size();
    vec2_t camera_pos = state->camera.position;
    camera_pos.x -= state->camera.shift.x;
    camera_pos.y -= state->camera.shift.y;

    for (unsigned int line = 0; line < screen.y; ++line)
    {
        for (unsigned int col = 0; col < screen.x; ++col)
        {
            if (((line + camera_pos.y) % grid.y) == 0
                && ((col + camera_pos.x) % grid.x) == 0)
            {
                display_set_char(&state->display, U'+', (disp_pos_t){col, line});
            }
        }
    }
    printf(HOME"camera_pos:%lld, %lld         \n", camera_pos.x, camera_pos.y);
    printf("last_mouse_position:%d, %d            \n", state->last_mouse_position.x, state->last_mouse_position.y);
    printf("press:%d, %d, %x       \n", state->mouse_pressed.position.x, state->mouse_pressed.position.y, state->mouse_pressed.type);
    printf("release:%d, %d, %x        \n", state->mouse_released.position.x, state->mouse_released.position.y, state->mouse_released.type);
}

static void render_objects(tifc_state_t *state)
{
    size_t size = dynarr_size(state->objects);
    for (size_t i = 0; i < size; ++i)
    {
        tifc_object_t *object = dynarr_get(state->objects, i);
        state->interfaces[object->interface].render(state, object);
    }
}

void tifc_init(tifc_state_t *state)
{
    state->objects = dynarr_create(.element_size = sizeof(tifc_object_t));
    state->interfaces[0] = (tifc_interface_t){
        .mouse_button = {
            [MOUSE_1_PRESSED] = text_area_mouse_1,
            [MOUSE_2_PRESSED] = text_area_mouse_2,
            [MOUSE_3_PRESSED] = text_area_mouse_3,
        },
        .render = text_area_render
    };
}

void tifc_delete(tifc_state_t *state)
{
    dynarr_destroy(state->objects);
}


static void display_swap_buffers(display_t *const display)
{
    display->active = (display->active + 1) % DISP_BUFFERS;
}

static int prev_buffer(const int active)
{
    return (active + DISP_BUFFERS - 1) % DISP_BUFFERS;
}

static void display_render(display_t *const display)
{
    disp_pos_t screen = tifc_get_window_size();
    dispbuf_ptr_t active = display->buffers[display->active];
    dispbuf_ptr_t previous = display->buffers[prev_buffer(display->active)];

    for (unsigned int line = 0; line < screen.y; ++line)
    {
        for (unsigned int col = 0; col < screen.x; ++col)
        {
            if (active[line][col] != previous[line][col])
            {
                fprintf(stderr, "\x1b[%d;%dH%lc", line, col, active[line][col]);
            }
        }
    }

    display_swap_buffers(display);
}

static void display_set_char(display_t *const display, wint_t ch, disp_pos_t pos)
{
    display->buffers[display->active][pos.y][pos.x] = ch;
}


static void display_clear(display_t *const display)
{
    wmemset(display->buffers[display->active][0], U' ', DISP_MAX_WIDTH * DISP_MAX_HEIGHT);
}


static void handle_mouse(tifc_state_t *tifc, unsigned char *input_buffer)
{
    mouse_event_t event = {
        .type = input_buffer[3],
        .position = {
            input_buffer[4] - MOUSE_POS_OFFSET,
            input_buffer[5] - MOUSE_POS_OFFSET
        }
    };

    // fprintf(stderr, HOME "\x1b[1B" "MOUSE DETECTED! (");
    // fprintf(stderr, "event: %X x: %d y: %d)\n", event.type, event.position.x, event.position.y);
    
    if (MOUSE_SCROLL_UP == event.type || MOUSE_SCROLL_DOWN == event.type)
    {
        // TODO: implement scroll
        return;
    }

    int mouse_button = MOUSE_BUTTON(event.type);
    if ( MOUSE_1_PRESSED == mouse_button
      || MOUSE_2_PRESSED == mouse_button
      || MOUSE_3_PRESSED == mouse_button) /* key pressed */
    {
        if (!tifc->mouse_pressed.type)
        {
            tifc->mouse_pressed = event;
        }
    }
    else // released
    {
        if (tifc->mouse_pressed.type && !tifc->mouse_released.type)
        {
            tifc->mouse_released = event;
        }
    }

    tifc->last_mouse_position = event.position;
}

void tifc_update_state(tifc_state_t *state)
{
    // void *target = get_target_object(state);
    
    if (state->type == TIFC_STATE_INSERT)
    {
        handle_text_edit(state);
    }
    else if (state->mouse_pressed.type)
    {
        handle_objects(state);
        handle_camera(state);
        handle_selection(state);
    }


    // clear stored events 
    if (state->mouse_pressed.type && state->mouse_released.type)
    {
        state->mouse_pressed.type = state->mouse_released.type = 0;
    }
}

static void handle_keyboard(tifc_state_t *state, unsigned char *input_buffer, const size_t bytes)
{
    // for (size_t i = 0; i < bytes; ++i)
    // {
    //     switch (input_buffer[i])
    //     {
    //         case 'D': break; // DELETE 
    //     }
    // }
    //

    if (TIFC_STATE_INSERT == state->type) // insert
    {
        if (input_buffer[0] == '\x1b' && input_buffer[1] == '\x5b') // ESCAPE - exit type mode
        {
            state->type = TIFC_STATE_HOVER;
            return;
        }
        
        printf("%x %x\n", input_buffer[0], input_buffer[1]);
        // TYPE TEXT
        text_area_t *area = state->insert_target->data;

        if (input_buffer[0] == '\x7f' && area->cursor > 0)
        {
            area->text[--area->cursor] = ' ';
            return;
        }

        if (area->cursor + bytes >= area->size)
        {
            area = resize_text_area(area, area->size * 2);
            if (area) state->insert_target->data = area;
            else exit(EXIT_FAILURE);
        }
        for (size_t i = 0; i < bytes; ++i)
        {
            area->text[i + area->cursor] = input_buffer[i];
        }
        area->cursor += bytes;
    }
}

static void handle_text_edit(tifc_state_t *state)
{
    /* for mouse while editing */
}

static disp_area_t normalized_area(disp_area_t area)
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

static tifc_object_t *get_target_object(tifc_state_t *state)
{
    vec2_t mouse_location = {
        (long long)state->camera.position.x + state->last_mouse_position.x,
        (long long)state->camera.position.y + state->last_mouse_position.y
    };

    size_t size = dynarr_size(state->objects);
    for (size_t i = 0; i < size; ++i)
    {
        tifc_object_t *object = dynarr_get(state->objects, i);

        if (mouse_location.x >= object->location.x
          && mouse_location.x < object->location.x + object->box.x
          && mouse_location.y >= object->location.y
          && mouse_location.y < object->location.y + object->box.y)
        {
            return object;
        }
    }
    return NULL;
}

static bool disp_pos_equal(disp_pos_t a, disp_pos_t b)
{
    return a.x == b.x && a.y == b.y;
}

static text_area_t *create_text_area(size_t size)
{
    text_area_t *area = malloc(sizeof(text_area_t) + sizeof(wchar_t) * size);
    if (area)
    {
        area->size = size;
        area->cursor = 0;
        area->text[0] = '\0';
    }
    return area;
}

static text_area_t *resize_text_area(text_area_t *area, size_t size)
{
    text_area_t *new = realloc(area, sizeof(text_area_t) + sizeof(wchar_t) * size);
    if (new)
    {
        new->size = size;
    }
    return new;
}

static void text_area_mouse_1(tifc_state_t *state, tifc_object_t *object)
{
    int button_idx = MOUSE_BUTTON(state->mouse_pressed.type);
    if (MOUSE_1_PRESSED != button_idx) return;

    state->type = TIFC_STATE_MOVE;
    object->shift = (vec2_t){
        (long long)state->last_mouse_position.x - state->mouse_pressed.position.x,
        (long long)state->last_mouse_position.y - state->mouse_pressed.position.y
    };

    if (state->mouse_released.type)
    {
        if (disp_pos_equal(state->mouse_pressed.position, state->mouse_released.position))
        {
            state->type = TIFC_STATE_INSERT; // ? INPUT TEXT
            state->insert_target = object;
            text_area_t *area = object->data;
            area->cursor = area->size - 1;
            // wmemcpy(area->text, L"Buy, World!", sizeof(wchar_t) * 12);
            return;
        }

        object->location.x += object->shift.x;
        object->location.y += object->shift.y;

        object->shift = (vec2_t) {0};
    }
}

static void text_area_mouse_2(tifc_state_t *state, tifc_object_t *object)
{

}

static void text_area_mouse_3(tifc_state_t *state, tifc_object_t *object)
{

}


static void text_area_render(tifc_state_t *state, tifc_object_t *object)
{
    disp_pos_t screen = tifc_get_window_size();
    vec2_t camera_pos = state->camera.position;
    camera_pos.x -= state->camera.shift.x;
    camera_pos.y -= state->camera.shift.y;

    text_area_t *text_area = object->data;
    vec2_t beg = {
        object->location.x - camera_pos.x + object->shift.x,
        object->location.y - camera_pos.y + object->shift.y,
    };
    vec2_t end = {
        beg.x + object->box.x - 1,
        beg.y + object->box.y - 1
    };

    size_t i = 0;
    wchar_t text_char = U' ';
    for (long long y = beg.y; y <= end.y; ++y)
    {
        bool next_line = false;
        for (long long x = beg.x; x <= end.x; ++x)
        {
            // read text
            if (x > beg.x + 1 && y > beg.y // (1, 0) gap
                && !next_line // wait for next line
                && i < text_area->size // src ended
                && U'\0' != text_char)
            {
                text_char = text_area->text[i++]; // read text char
                if (U'\n' == text_char) next_line = true;
            }

            // render
            if (x >= 0 && y >= 0 && x < screen.x && y < screen.y) // inside screen
            {
                disp_pos_t pos = {x, y};
                display_set_char(&state->display, U' ', pos);
                if (x == beg.x && y == beg.y) display_set_char(&state->display, U'╔', pos);
                else if (x == end.x && y == beg.y) display_set_char(&state->display, U'╗', pos);
                else if (x == end.x && y == end.y) display_set_char(&state->display, U'╝', pos);
                else if (x == beg.x && y == end.y) display_set_char(&state->display, U'╚', pos);
                else if (x == beg.x || x == end.x) display_set_char(&state->display, U'║', pos);
                else if (y == beg.y || y == end.y) display_set_char(&state->display, U'═', pos);
                else if (!next_line && text_char != '\0')
                {
                   display_set_char(&state->display, text_char, pos);
                }
            }
        }
    }
}


int main(void)
{
    tifc_state_t state = {0};
    tifc_init(&state);

    // TEST FOR TEXT AREA
    tifc_object_t text_demo = {
        .location = {.x = 0, .y = 0},
        .box = {DEFAULT_TEXT_AREA_WIDTH, DEFAULT_TEXT_AREA_HEIGHT},
        .interface = 0,
        .data = create_text_area(20)
    };
    wmemcpy(((text_area_t*)text_demo.data)->text, L"Hello, World!", 14);
    dynarr_append(&state.objects, &text_demo);


    setlocale(LC_ALL, "");
    tifc_mouse_begin();

    while (state.input_buffer[0] != 'Q')
    {
        size_t bytes = read(fileno(stdin), state.input_buffer, INPUT_BUFFER_SIZE);
        if (bytes == INPUT_BUFFER_SIZE && 0 == memcmp(state.input_buffer, "\x1b[M", 3))
        {
            handle_mouse(&state, state.input_buffer);
        }
        else
        {
            handle_keyboard(&state, state.input_buffer, bytes);
        }

        tifc_update_state(&state);
        render_screen(&state);
    }
    
    tifc_mouse_end();
    tifc_delete(&state);
}
