/*++
Copyright (c) Microsoft Corporation
Licensed under the MIT license.

Module Name:
- AccessibilityNotifier.hpp

Abstract:
- Win32 implementation of the AccessibilityNotifier interface.

Author(s):
- Hernan Gatta (HeGatta) 29-Mar-2017
--*/

#pragma once

namespace Microsoft::Console::Interactivity
{
    enum class ConsoleCaretEventFlags
    {
        CaretInvisible,
        CaretSelection,
        CaretVisible
    };
}

namespace Microsoft::Console::Interactivity::Win32
{
    class AccessibilityNotifier
    {
    public:
        void NotifyConsoleCaretEvent(_In_ RECT rectangle);
        void NotifyConsoleCaretEvent(_In_ ConsoleCaretEventFlags flags, _In_ LONG position);
        void NotifyConsoleUpdateScrollEvent(_In_ LONG x, _In_ LONG y);
        void NotifyConsoleUpdateSimpleEvent(_In_ LONG start, _In_ LONG charAndAttribute);
        void NotifyConsoleUpdateRegionEvent(_In_ LONG startXY, _In_ LONG endXY);
        void NotifyConsoleLayoutEvent();
        void NotifyConsoleStartApplicationEvent(_In_ DWORD processId);
        void NotifyConsoleEndApplicationEvent(_In_ DWORD processId);
    };
}
