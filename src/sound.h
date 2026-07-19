// sound.h - procedural WAV synthesis (sound.c). Five period effects are
// built from triangle waves and noise in integer math at startup and played
// from memory; no asset files, no floating point.

#ifndef GANGLAND_SOUND_H
#define GANGLAND_SOUND_H

#include "gangland.h"

void SoundBuild(App* app);
void SoundPlay(App* app, int kind);

#endif
