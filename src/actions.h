// actions.h - executes whichever choice the player picked (actions.c).
// HandleAction unpacks the code + argument packed by PACK_ACTION.

#ifndef GANGLAND_ACTIONS_H
#define GANGLAND_ACTIONS_H

#include "gangland.h"

void HandleAction(App* app, int packedAction);

#endif
