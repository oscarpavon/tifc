#ifndef _ECS_SYSTEM_H_
#define _ECS_SYSTEM_H_

#include "ecs_component.h"
#include "dynarr.h"

typedef struct ecs_system
{
    dynarr_t *components; /* might be sorted by z-index */
}
ecs_system_t;



#endif//_ECS_SYSTEM_H_
