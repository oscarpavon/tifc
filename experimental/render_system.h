#ifndef _RENDER_SYSTEM_H_
#define _RENDER_SYSTEM_H_

#include "ecs_system.h"
#include "display.h"

/*
 * Renders whole application
 */
void rendsys_render();

/*
 * Create new render component
 */
component_id rendsys_new_component();


#endif//_RENDER_SYSTEM_H_
