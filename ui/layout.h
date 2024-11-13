#ifndef _LAYOUT_H_
#define _LAYOUT_H_

#include "display.h"

typedef enum
{
    LAYOUT_ALIGN_TOP            = 1 << 0,
    LAYOUT_ALIGN_BOT            = 1 << 1,
    LAYOUT_ALIGN_LEFT           = 1 << 2,
    LAYOUT_ALIGN_RIGHT          = 1 << 3,

    LAYOUT_ALIGN_TOP_H_CENTER   = LAYOUT_ALIGN_TOP
                                | LAYOUT_ALIGN_LEFT
                                | LAYOUT_ALIGN_RIGHT,
    
    LAYOUT_ALIGN_BOT_H_CENTER   = LAYOUT_ALIGN_BOT
                                | LAYOUT_ALIGN_LEFT
                                | LAYOUT_ALIGN_RIGHT,
    
    LAYOUT_ALIGN_LEFT_V_CENTER  = LAYOUT_ALIGN_LEFT
                                | LAYOUT_ALIGN_TOP
                                | LAYOUT_ALIGN_BOT,
    
    LAYOUT_ALIGN_RIGHT_V_CENTER = LAYOUT_ALIGN_RIGHT
                                | LAYOUT_ALIGN_TOP
                                | LAYOUT_ALIGN_BOT,

    LAYOUT_ALIGN_CENTER         = LAYOUT_ALIGN_LEFT
                                | LAYOUT_ALIGN_RIGHT
                                | LAYOUT_ALIGN_TOP
                                | LAYOUT_ALIGN_BOT,
}
layout_align_t;

typedef enum
{
    LAYOUT_CONTENT_DISTRIBUTE = 0,
    LAYOUT_CONTENT_CENTER,
    LAYOUT_CONTENT_START,
    LAYOUT_CONTENT_END,
}
layout_content_t;

typedef enum
{
    LAYOUT_DIR_HORIZONTAL = 0,
    LAYOUT_DIR_VERTICAL,
}
layout_dir_t;

typedef struct
{
    layout_dir_t    dir;
    layout_align_t  align;
    disp_pos_t      min;
    disp_pos_t      max;
}
layout_t;

#endif /* _LAYOUT_H_ */
