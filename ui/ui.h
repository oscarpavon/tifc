#ifndef _UI_H_
#define _UI_H_

#include "layout.h"
#include "input.h"
#include "sparse.h"

typedef struct
{
    input_hooks_t hooks;
    sparse_t     *panels;
    size_t        last_id;
}
ui_t;

ui_t ui_init(void);
void ui_deinit(ui_t *const ui);
void ui_render(const ui_t *const ui, display_t *const display);
void ui_add_panel(ui_t *const ui, const char *title, layout_t layout, style_t style);

#endif /* _UI_H_ */
