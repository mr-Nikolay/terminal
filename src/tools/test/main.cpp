// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include <precomp.h>
#include <windows.h>
#include <wincon.h>

int TestSetViewport(HANDLE hIn, HANDLE hOut);
int TestGetchar(HANDLE hIn, HANDLE hOut);

int __cdecl wmain(int /*argc*/, WCHAR* /*argv[]*/)
{
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    TestGetchar(hIn, hOut);

    return 0;
}

int TestSetViewport(HANDLE /*hIn*/, HANDLE hOut)
{
    CONSOLE_SCREEN_BUFFER_INFOEX csbiex = { 0 };
    csbiex.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
    bool fSuccess = GetConsoleScreenBufferInfoEx(hOut, &csbiex);
    if (fSuccess)
    {
        const auto Screen = csbiex.srWindow;
        const auto sWidth = Screen.right - Screen.left;
        const auto sHeight = Screen.bottom - Screen.top;

        csbiex.srWindow.top = 50;
        csbiex.srWindow.bottom = sHeight + 50;
        csbiex.srWindow.left = 0;
        csbiex.srWindow.right = sWidth;

        SetConsoleScreenBufferInfoEx(hOut, &csbiex);
    }
    return 0;
}

int TestGetchar(HANDLE hIn, HANDLE /*hOut*/)
{
    DWORD dwInputModes;
    if (!GetConsoleMode(hIn, &dwInputModes))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    DWORD const dwEnableVirtualTerminalInput = 0x200; // Until the new wincon.h is published
    if (!SetConsoleMode(hIn, dwInputModes | dwEnableVirtualTerminalInput))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    while (hIn != nullptr)
    {
        int ch = getchar();
        printf("0x%x\r\n", ch);
    }

    return 0;
}
