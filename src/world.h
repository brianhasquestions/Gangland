// world.h - game-state lifecycle (world.c): new game, the end-of-day cycle
// with all its night events, persistence, and shared world queries.
// Loaded saves pass through sanitization that clamps every index and
// terminates every string before the bytes are trusted.

#ifndef GANGLAND_WORLD_H
#define GANGLAND_WORLD_H

#include "gangland.h"

void NewGame(App* app);
void EndDay(App* app);                             // income, wages, night events
int PeekSave(int slot, GameState* peek);           // read a save without loading
void RollHideout(GameState* game, int rivalIndex); // (re)populate a hideout
void AddHeat(App* app, int amount);                // scaled by local police
void GainBusinessXp(App* app, int amount);
void GainGunplayXp(App* app, int amount);
void BuildStatusText(App* app);
void SaveGame(App* app, int slot, int silent);
void LoadGame(App* app, int slot);
const WCHAR* DistrictName(int district);
const UnitSpec* GetUnitSpec(int type);
int CountCrew(GameState* game);
int LeadershipCap(GameState* game);                // how many you can lead
int ControlsBusinessType(GameState* game, int type);
void RecruitUnit(App* app, int type, int silent);
const WCHAR* KidnapVictimName(GameState* game);
void KidnapVictimDies(App* app);
void TagMap(App* app, int district, int kind);     // stamp a map ping
Rival* RivalInDistrict(GameState* game, int district);
void RaiseRivalAnger(App* app, int district, int amount);

#endif
