#include "gangland.h"

void WinMainCRTStartup(void)
{
    int exitCode = 0;

    exitCode = RunApp();
    ExitProcess((UINT)exitCode);
}
