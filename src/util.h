// util.h - allocation, randomness, and bounded string copies (util.c).
// Also home to the no-CRT memset/memcpy the compiler is allowed to call.

#ifndef GANGLAND_UTIL_H
#define GANGLAND_UTIL_H

#include "gangland.h"

void* AllocZeroed(SIZE_T bytes);
void FreeMemory(void* memory);
int RandomRange(GameState* game, int span);            // 0 .. span-1
int RandomBetween(GameState* game, int low, int high); // inclusive
void CopyText(WCHAR* dest, const WCHAR* source, int destChars);

#endif
