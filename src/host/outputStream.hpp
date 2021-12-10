/*++
Copyright (c) Microsoft Corporation
Licensed under the MIT license.

Module Name:
- outputStream.hpp

Abstract:
- Classes to process text written into the console on the attached application's output stream (usually STDOUT).

Author:
- Michael Niksa <miniksa> July 27 2015
--*/

#pragma once

#include "../terminal/adapter/adaptDefaults.hpp"
#include "../types/inc/IInputEvent.hpp"
#include "../inc/conattrs.hpp"
#include "IIoProvider.hpp"

class SCREEN_INFORMATION;

// The WriteBuffer class provides helpers for writing text into the TextBuffer that is backing a particular console screen buffer.
class WriteBuffer : public Microsoft::Console::VirtualTerminal::AdaptDefaults
{
public:
    WriteBuffer(_In_ Microsoft::Console::IIoProvider& io);

    // Implement Adapter callbacks for default cases (non-escape sequences)
    void Print(const wchar_t wch) override;
    void PrintString(const std::wstring_view string) override;
    void Execute(const wchar_t wch) override;

    [[nodiscard]] NTSTATUS GetResult() { return _ntstatus; };

private:
    void _DefaultCase(const wchar_t wch);
    void _DefaultStringCase(const std::wstring_view string);

    Microsoft::Console::IIoProvider& _io;
    NTSTATUS _ntstatus;
};

#include "../terminal/adapter/conGetSet.hpp"

// The ConhostInternalGetSet is for the Conhost process to call the entrypoints for its own Get/Set APIs.
// Normally, these APIs are accessible from the outside of the conhost process (like by the process being "hosted") through
// the kernelbase/32 exposed public APIs and routed by the console driver (condrv) to this console host.
// But since we're trying to call them from *inside* the console host itself, we need to get in the way and route them straight to the
// v-table inside this process instance.
class ConhostInternalGetSet final : public Microsoft::Console::VirtualTerminal::ConGetSet
{
public:
    ConhostInternalGetSet(_In_ Microsoft::Console::IIoProvider& io);

    bool GetConsoleScreenBufferInfoEx(CONSOLE_SCREEN_BUFFER_INFOEX& screenBufferInfo) const override;
    bool SetConsoleScreenBufferInfoEx(const CONSOLE_SCREEN_BUFFER_INFOEX& screenBufferInfo) override;

    bool SetConsoleCursorPosition(const til::point position) override;

    bool PrivateGetTextAttributes(TextAttribute& attrs) const override;
    bool PrivateSetTextAttributes(const TextAttribute& attrs) override;

    bool PrivateSetCurrentLineRendition(const LineRendition lineRendition) override;
    bool PrivateResetLineRenditionRange(const til::CoordType startRow, const til::CoordType endRow) override;
    til::CoordType PrivateGetLineWidth(const til::CoordType row) const override;

    bool PrivateWriteConsoleInputW(std::deque<std::unique_ptr<IInputEvent>>& events,
                                   size_t& eventsWritten) override;

    bool SetConsoleWindowInfo(bool const absolute,
                              const til::inclusive_rect window) override;

    bool SetInputMode(const Microsoft::Console::VirtualTerminal::TerminalInput::Mode mode, const bool enabled) override;
    bool SetParserMode(const Microsoft::Console::VirtualTerminal::StateMachine::Mode mode, const bool enabled) override;
    bool GetParserMode(const Microsoft::Console::VirtualTerminal::StateMachine::Mode mode) const override;

    bool PrivateSetScreenMode(const bool reverseMode) override;
    bool PrivateSetAutoWrapMode(const bool wrapAtEOL) override;

    bool PrivateShowCursor(const bool show) noexcept override;
    bool PrivateAllowCursorBlinking(const bool enable) override;

    bool PrivateSetScrollingRegion(const til::inclusive_rect scrollMargins) override;

    bool PrivateWarningBell() override;

    bool PrivateGetLineFeedMode() const override;
    bool PrivateLineFeed(const bool withReturn) override;
    bool PrivateReverseLineFeed() override;

    bool SetConsoleTitleW(const std::wstring_view title) override;

    bool PrivateUseAlternateScreenBuffer() override;

    bool PrivateUseMainScreenBuffer() override;

    bool PrivateEraseAll() override;
    bool PrivateClearBuffer() override;

    bool GetUserDefaultCursorStyle(CursorType& style) override;
    bool SetCursorStyle(CursorType const style) override;

    bool PrivateRefreshWindow() override;

    bool PrivateSuppressResizeRepaint() override;

    bool PrivateWriteConsoleControlInput(const KeyEvent key) override;

    bool SetConsoleOutputCP(const unsigned int codepage) override;
    bool GetConsoleOutputCP(unsigned int& codepage) override;

    bool IsConsolePty() const override;

    bool DeleteLines(const til::CoordType count) override;
    bool InsertLines(const til::CoordType count) override;

    bool MoveToBottom() const override;

    COLORREF GetColorTableEntry(const size_t tableIndex) const noexcept override;
    bool SetColorTableEntry(const size_t tableIndex, const COLORREF color) noexcept override;

    bool PrivateFillRegion(const til::point startPosition,
                           const size_t fillLength,
                           const wchar_t fillChar,
                           const bool standardFillAttrs) noexcept override;

    bool PrivateScrollRegion(const til::inclusive_rect scrollRect,
                             const std::optional<til::inclusive_rect> clipRect,
                             const til::point destinationOrigin,
                             const bool standardFillAttrs) noexcept override;

    bool PrivateIsVtInputEnabled() const override;

    bool PrivateAddHyperlink(const std::wstring_view uri, const std::wstring_view params) const override;
    bool PrivateEndHyperlink() const override;

    bool PrivateUpdateSoftFont(const gsl::span<const uint16_t> bitPattern,
                               const til::size cellSize,
                               const size_t centeringHint) noexcept override;

private:
    Microsoft::Console::IIoProvider& _io;
};
