/*++
Copyright (c) Microsoft Corporation
Licensed under the MIT license.

Module Name:
- conGetSet.hpp

Abstract:
- This serves as an abstraction layer for the adapters to connect to the console API functions.
- The abstraction allows for the substitution of the functions for internal/external to Conhost.exe use as well as easy testing.

Author(s):
- Michael Niksa (MiNiksa) 30-July-2015
--*/

#pragma once

#include "../input/terminalInput.hpp"
#include "../parser/stateMachine.hpp"
#include "../../types/inc/IInputEvent.hpp"
#include "../../buffer/out/LineRendition.hpp"
#include "../../buffer/out/TextAttribute.hpp"
#include "../../inc/conattrs.hpp"

#include <deque>
#include <memory>

namespace Microsoft::Console::VirtualTerminal
{
    class ConGetSet
    {
    public:
        virtual ~ConGetSet() = default;
        virtual bool GetConsoleScreenBufferInfoEx(CONSOLE_SCREEN_BUFFER_INFOEX& screenBufferInfo) const = 0;
        virtual bool SetConsoleScreenBufferInfoEx(const CONSOLE_SCREEN_BUFFER_INFOEX& screenBufferInfo) = 0;
        virtual bool SetConsoleCursorPosition(const til::point position) = 0;

        virtual bool PrivateIsVtInputEnabled() const = 0;

        virtual bool PrivateGetTextAttributes(TextAttribute& attrs) const = 0;
        virtual bool PrivateSetTextAttributes(const TextAttribute& attrs) = 0;

        virtual bool PrivateSetCurrentLineRendition(const LineRendition lineRendition) = 0;
        virtual bool PrivateResetLineRenditionRange(const til::CoordType startRow, const til::CoordType endRow) = 0;
        virtual til::CoordType PrivateGetLineWidth(const til::CoordType row) const = 0;

        virtual bool PrivateWriteConsoleInputW(std::deque<std::unique_ptr<IInputEvent>>& events,
                                               size_t& eventsWritten) = 0;
        virtual bool SetConsoleWindowInfo(const bool absolute,
                                          const til::inclusive_rect window) = 0;

        virtual bool SetInputMode(const TerminalInput::Mode mode, const bool enabled) = 0;
        virtual bool SetParserMode(const StateMachine::Mode mode, const bool enabled) = 0;
        virtual bool GetParserMode(const StateMachine::Mode mode) const = 0;

        virtual bool PrivateSetScreenMode(const bool reverseMode) = 0;
        virtual bool PrivateSetAutoWrapMode(const bool wrapAtEOL) = 0;

        virtual bool PrivateShowCursor(const bool show) = 0;
        virtual bool PrivateAllowCursorBlinking(const bool enable) = 0;

        virtual bool PrivateSetScrollingRegion(const til::inclusive_rect scrollMargins) = 0;
        virtual bool PrivateWarningBell() = 0;
        virtual bool PrivateGetLineFeedMode() const = 0;
        virtual bool PrivateLineFeed(const bool withReturn) = 0;
        virtual bool PrivateReverseLineFeed() = 0;
        virtual bool SetConsoleTitleW(const std::wstring_view title) = 0;
        virtual bool PrivateUseAlternateScreenBuffer() = 0;
        virtual bool PrivateUseMainScreenBuffer() = 0;

        virtual bool PrivateEraseAll() = 0;
        virtual bool PrivateClearBuffer() = 0;
        virtual bool GetUserDefaultCursorStyle(CursorType& style) = 0;
        virtual bool SetCursorStyle(const CursorType style) = 0;
        virtual bool PrivateWriteConsoleControlInput(const KeyEvent key) = 0;
        virtual bool PrivateRefreshWindow() = 0;

        virtual bool SetConsoleOutputCP(const unsigned int codepage) = 0;
        virtual bool GetConsoleOutputCP(unsigned int& codepage) = 0;

        virtual bool PrivateSuppressResizeRepaint() = 0;
        virtual bool IsConsolePty() const = 0;

        virtual bool DeleteLines(const til::CoordType count) = 0;
        virtual bool InsertLines(const til::CoordType count) = 0;

        virtual bool MoveToBottom() const = 0;

        virtual COLORREF GetColorTableEntry(const size_t tableIndex) const = 0;
        virtual bool SetColorTableEntry(const size_t tableIndex, const COLORREF color) = 0;

        virtual bool PrivateFillRegion(const til::point startPosition,
                                       const size_t fillLength,
                                       const wchar_t fillChar,
                                       const bool standardFillAttrs) = 0;

        virtual bool PrivateScrollRegion(const til::inclusive_rect scrollRect,
                                         const std::optional<til::inclusive_rect> clipRect,
                                         const til::point destinationOrigin,
                                         const bool standardFillAttrs) = 0;

        virtual bool PrivateAddHyperlink(const std::wstring_view uri, const std::wstring_view params) const = 0;
        virtual bool PrivateEndHyperlink() const = 0;

        virtual bool PrivateUpdateSoftFont(const gsl::span<const uint16_t> bitPattern,
                                           const til::size cellSize,
                                           const size_t centeringHint) = 0;
    };
}
