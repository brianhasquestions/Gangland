// main.h - the window and everything drawn in it (main.c).
// Owns the message loop, the four panes, the map and family-strip painting,
// and the log/choice plumbing every other module talks to the player through.

#ifndef GANGLAND_MAIN_H
#define GANGLAND_MAIN_H

#include "gangland.h"

int RunApp(void);                                  // called by entry.c
void UiLog(App* app, const WCHAR* text);           // append a line to the log
void UiLogFmt(App* app, const WCHAR* format, ...); // printf-style append
void RefreshUi(App* app);                          // rebuild status + choices
void AddChoice(App* app, const WCHAR* label, int packedAction);

#endif
