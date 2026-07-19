// menu.h - builds the choice list for whichever menu screen is active
// (menu.c). Reads GameState.menuId and fills the listbox via AddChoice.

#ifndef GANGLAND_MENU_H
#define GANGLAND_MENU_H

#include "gangland.h"

void BuildChoices(App* app);

#endif
