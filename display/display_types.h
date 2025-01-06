#ifndef _DISPLAY_TYPES_
#define _DISPLAY_TYPES_

#include <stdint.h>

/* Describes position of the character on the screen */
typedef struct
{
    uint16_t x;
    uint16_t y;
}
disp_pos_t;

/* Describes are of the screen */
typedef struct
{
    disp_pos_t first;  /* left-top corner     */
    disp_pos_t second; /* right-bottom corner */
}
disp_area_t;

#endif//_DISPLAY_TYPES_
