#include "ui.h"
#include "panel.h"
#include "display.h"
#include "sparse.h"

#include <stdio.h>
#include <stdlib.h>

//
// Mouse events
//
static void on_hover(const mouse_event_t *const, void *const);
static void on_press(const mouse_event_t *const, void *const);
static void on_release(const mouse_event_t *const, void *const);
static void on_drag_begin(const mouse_event_t *const, void *const);
static void on_drag(const mouse_event_t *const, const mouse_event_t *const, void *const);
static void on_drag_end(const mouse_event_t *const, const mouse_event_t *const, void *const);
static void on_scroll(const mouse_event_t *const, void *const);

static input_hooks_t hooks_init(void)
{
    return (input_hooks_t)
    {
        .on_hover = on_hover,
        .on_press = on_press,
        .on_release = on_release,
        .on_drag_begin = on_drag_begin,
        .on_drag = on_drag,
        .on_drag_end = on_drag_end,
        .on_scroll = on_scroll
    };
}

ui_t ui_init(void)
{
    sparse_t *panels = sparse_create(
        .element_size = sizeof(panel_t)
    );

    if (!panels)
    {
        exit(EXIT_FAILURE);
    }

    return (ui_t) {
        .panels = panels,
        .hooks = hooks_init(),
    };
}

void ui_deinit(ui_t *const ui)
{
    sparse_destroy(ui->panels);
}

void ui_render(const ui_t *const ui,
               display_t *const display)
{
    (void) ui;
    // TODO 

    screen_space_bounds_t bounds = {
        .first = {0, 0},
        .second = {display->size.x - 1, display->size.y - 1}
    };

    size_t size = sparse_size(ui->panels);
    for (size_t i = 0; i < size; ++i)
    {
        panel_t *panel = sparse_get(ui->panels, i);
        if (panel)
        {
            panel_render(panel, display, &bounds);
        }
    }
    display_set_char(display, U'U', (disp_pos_t){.x=100,.y=0});
    display_set_char(display, U'I', (disp_pos_t){.x=101,.y=0});
}

void ui_add_panel(ui_t *const ui,
                  const char *title,
                  layout_t layout,
                  style_t style)
{
    (void) sparse_insert_reserve(&ui->panels, ui->last_id++);
    panel_t *panel = sparse_get(ui->panels, ui->last_id - 1);
    *panel = (panel_t){
        .title = title,
        .layout = layout,
        .style = style,
    };
}

static void on_hover(const mouse_event_t *const hover, void *const param)
{
    (void) param;
    printf(ROW(1) "UI::hover, at %u, %u\n",
        hover->position.x, hover->position.y);
}

static void on_press(const mouse_event_t *const press, void *const param)
{
    (void) param;
    printf(ROW(1) "UI::press %d, at %u, %u\n",
        press->mouse_button,
        press->position.x, press->position.y);
}

static void on_release(const mouse_event_t *const press, void *const param)
{
    (void) param;
    printf(ROW(1) "UI::release %d, at %u, %u\n",
        press->mouse_button,
        press->position.x, press->position.y);
}

static void on_drag_begin(const mouse_event_t *const begin,
        void *const param)
{
    (void) param;
    printf(ROW(1) "UI::drag %d begin, at %u, %u\n",
        begin->mouse_button,
        begin->position.x, begin->position.y);
}

static void on_drag(const mouse_event_t *const begin, const mouse_event_t *const moved, void *const param)
{
    (void) param;
    printf(ROW(1) "UI::drag %d drag moving to %u, %u\n",
        begin->mouse_button,
        moved->position.x, moved->position.y);
}

static void on_drag_end(const mouse_event_t *const begin,
        const mouse_event_t *const end, void *const param)
{
    (void) param;
    printf(ROW(1) "UI::drag %d from %u, %u to %u, %u\n",
        begin->mouse_button,
        begin->position.x, begin->position.y,
        end->position.x, end->position.y);
}

static void on_scroll(const mouse_event_t *const scroll, void *const param)
{
    (void) param;
    printf(ROW(1) "UI::scroll %d at %u, %u\n",
        scroll->mouse_button,
        scroll->position.x, scroll->position.y);
}
