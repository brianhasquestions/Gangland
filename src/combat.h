// combat.h - the battle engine (combat.c): offensive fights staged through
// the approach menu, defensive night raids, and everything a victory or
// defeat means (loot, succession, casualties, experience).

#ifndef GANGLAND_COMBAT_H
#define GANGLAND_COMBAT_H

#include "gangland.h"

void QueueCombat(App* app, const WCHAR* label);    // pending -> approach menu
void ResolveCombat(App* app, int approach);
void ResolveRaid(App* app, int kind);

#endif
