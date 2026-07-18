#include "gangland.h"
#include <stddef.h>
#include <intrin.h>

#define RNG_MULTIPLIER 1664525u
#define RNG_INCREMENT 1013904223u
#define RNG_SHIFT 16

void* __cdecl memset(void* dest, int value, size_t count);
#pragma function(memset)
void* __cdecl memset(void* dest, int value, size_t count)
{
    __stosb((unsigned char*)dest, (unsigned char)value, count);

    return dest;
}

void* __cdecl memcpy(void* dest, const void* source, size_t count);
#pragma function(memcpy)
void* __cdecl memcpy(void* dest, const void* source, size_t count)
{
    __movsb((unsigned char*)dest, (const unsigned char*)source, count);

    return dest;
}

void* AllocZeroed(SIZE_T bytes)
{
    void* memory = NULL;

    memory = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bytes);

    return memory;
}

void FreeMemory(void* memory)
{
    if (NULL != memory)
    {
        HeapFree(GetProcessHeap(), 0, memory);
    }
}

int RandomRange(GameState* game, int span)
{
    int result = 0;

    if (NULL == game || 0 >= span)
    {
        goto Cleanup;
    }
    game->rngSeed = (game->rngSeed * RNG_MULTIPLIER) + RNG_INCREMENT;
    result = (int)((game->rngSeed >> RNG_SHIFT) % (unsigned int)span);

Cleanup:

    return result;
}

int RandomBetween(GameState* game, int low, int high)
{
    int result = 0;

    result = low + RandomRange(game, (high - low) + 1);

    return result;
}

void CopyText(WCHAR* dest, const WCHAR* source, int destChars)
{
    if (NULL == dest || NULL == source || 0 >= destChars)
    {
        goto Cleanup;
    }
    lstrcpynW(dest, source, destChars);

Cleanup:

    return;
}
