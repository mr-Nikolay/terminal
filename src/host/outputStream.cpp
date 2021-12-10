// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "precomp.h"

#include "outputStream.hpp"

#include "_stream.h"
#include "getset.h"
#include "directio.h"

#include "../interactivity/inc/ServiceLocator.hpp"

#pragma hdrstop

using namespace Microsoft::Console;
using Microsoft::Console::Interactivity::ServiceLocator;
using Microsoft::Console::VirtualTerminal::StateMachine;
using Microsoft::Console::VirtualTerminal::TerminalInput;

WriteBuffer::WriteBuffer(_In_ Microsoft::Console::IIoProvider& io) :
    _io{ io },
    _ntstatus{ STATUS_INVALID_DEVICE_STATE }
{
}

// Routine Description:
// - Handles the print action from the state machine
// Arguments:
// - wch - The character to be printed.
// Return Value:
// - <none>
void WriteBuffer::Print(const wchar_t wch)
{
    _DefaultCase(wch);
}

// Routine Description:
// - Handles the print action from the state machine
// Arguments:
// - string - The string to be printed.
// Return Value:
// - <none>
void WriteBuffer::PrintString(const std::wstring_view string)
{
    _DefaultStringCase(string);
}

// Routine Description:
// - Handles the execute action from the state machine
// Arguments:
// - wch - The C0 control character to be executed.
// Return Value:
// - <none>
void WriteBuffer::Execute(const wchar_t wch)
{
    _DefaultCase(wch);
}

// Routine Description:
// - Default text editing/printing handler for all characters that were not routed elsewhere by other state machine intercepts.
// Arguments:
// - wch - The character to be processed by our default text editing/printing mechanisms.
// Return Value:
// - <none>
void WriteBuffer::_DefaultCase(const wchar_t wch)
{
    _DefaultStringCase({ &wch, 1 });
}

// Routine Description:
// - Default text editing/printing handler for all characters that were not routed elsewhere by other state machine intercepts.
// Arguments:
// - string - The string to be processed by our default text editing/printing mechanisms.
// Return Value:
// - <none>
void WriteBuffer::_DefaultStringCase(const std::wstring_view string)
{
    size_t dwNumBytes = string.size() * sizeof(wchar_t);

    Cursor& cursor = _io.GetActiveOutputBuffer().GetTextBuffer().GetCursor();
    if (!cursor.IsOn())
    {
        cursor.SetIsOn(true);
    }

    // Defer the cursor drawing while we are iterating the string, for a better performance.
    // We can not waste time displaying a cursor event when we know more text is coming right behind it.
    cursor.StartDeferDrawing();
    _ntstatus = WriteCharsLegacy(_io.GetActiveOutputBuffer(),
                                 string.data(),
                                 string.data(),
                                 string.data(),
                                 &dwNumBytes,
                                 nullptr,
                                 _io.GetActiveOutputBuffer().GetTextBuffer().GetCursor().GetPosition().x,
                                 WC_LIMIT_BACKSPACE | WC_DELAY_EOL_WRAP,
                                 nullptr);
    cursor.EndDeferDrawing();
}

ConhostInternalGetSet::ConhostInternalGetSet(_In_ IIoProvider& io) :
    _io{ io }
{
}

// Routine Description:
// - Connects the GetConsoleScreenBufferInfoEx API call directly into our Driver Message servicing call inside Conhost.exe
// Arguments:
// - screenBufferInfo - Structure to hold screen buffer information like the public API call.
// Return Value:
// - true if successful (see DoSrvGetConsoleScreenBufferInfo). false otherwise.
bool ConhostInternalGetSet::GetConsoleScreenBufferInfoEx(CONSOLE_SCREEN_BUFFER_INFOEX& screenBufferInfo) const
{
    ServiceLocator::LocateGlobals().api.GetConsoleScreenBufferInfoExImpl(_io.GetActiveOutputBuffer(), screenBufferInfo);
    return true;
}

// Routine Description:
// - Connects the SetConsoleScreenBufferInfoEx API call directly into our Driver Message servicing call inside Conhost.exe
// Arguments:
// - screenBufferInfo - Structure containing screen buffer information like the public API call.
// Return Value:
// - true if successful (see DoSrvSetConsoleScreenBufferInfo). false otherwise.
bool ConhostInternalGetSet::SetConsoleScreenBufferInfoEx(const CONSOLE_SCREEN_BUFFER_INFOEX& screenBufferInfo)
{
    return SUCCEEDED(ServiceLocator::LocateGlobals().api.SetConsoleScreenBufferInfoExImpl(_io.GetActiveOutputBuffer(), screenBufferInfo));
}

// Routine Description:
// - Connects the SetConsoleCursorPosition API call directly into our Driver Message servicing call inside Conhost.exe
// Arguments:
// - position - new cursor position to set like the public API call.
// Return Value:
// - true if successful (see DoSrvSetConsoleCursorPosition). false otherwise.
bool ConhostInternalGetSet::SetConsoleCursorPosition(const til::point position)
{
    auto& info = _io.GetActiveOutputBuffer();
    const auto clampedPosition = info.GetTextBuffer().ClampPositionWithinLine(position);
    return SUCCEEDED(ServiceLocator::LocateGlobals().api.SetConsoleCursorPositionImpl(info, clampedPosition));
}

// Method Description:
// - Retrieves the current TextAttribute of the active screen buffer.
// Arguments:
// - attrs: Receives the TextAttribute value.
// Return Value:
// - true if successful. false otherwise.
bool ConhostInternalGetSet::PrivateGetTextAttributes(TextAttribute& attrs) const
{
    attrs = _io.GetActiveOutputBuffer().GetAttributes();
    return true;
}

// Method Description:
// - Sets the current TextAttribute of the active screen buffer. Text
//   written to this buffer will be written with these attributes.
// Arguments:
// - attrs: The new TextAttribute to use
// Return Value:
// - true if successful. false otherwise.
bool ConhostInternalGetSet::PrivateSetTextAttributes(const TextAttribute& attrs)
{
    _io.GetActiveOutputBuffer().SetAttributes(attrs);
    return true;
}

// Method Description:
// - Sets the line rendition attribute for the current row of the active screen
//   buffer. This controls how character cells are scaled when the row is rendered.
// Arguments:
// - lineRendition: The new LineRendition attribute to use
// Return Value:
// - true if successful. false otherwise.
bool ConhostInternalGetSet::PrivateSetCurrentLineRendition(const LineRendition lineRendition)
{
    auto& textBuffer = _io.GetActiveOutputBuffer().GetTextBuffer();
    textBuffer.SetCurrentLineRendition(lineRendition);
    return true;
}

// Method Description:
// - Resets the line rendition attribute to SingleWidth for a specified range
//   of row numbers.
// Arguments:
// - startRow: The row number of first line to be modified
// - endRow: The row number following the last line to be modified
// Return Value:
// - true if successful. false otherwise.
bool ConhostInternalGetSet::PrivateResetLineRenditionRange(const til::CoordType startRow, const til::CoordType endRow)
{
    auto& textBuffer = _io.GetActiveOutputBuffer().GetTextBuffer();
    textBuffer.ResetLineRenditionRange(startRow, endRow);
    return true;
}

// Method Description:
// - Returns the number of cells that will fit on the specified row when
//   rendered with its current line rendition.
// Arguments:
// - row: The row number of the line to measure
// Return Value:
// - the number of cells that will fit on the line
til::CoordType ConhostInternalGetSet::PrivateGetLineWidth(const til::CoordType row) const
{
    const auto& textBuffer = _io.GetActiveOutputBuffer().GetTextBuffer();
    return textBuffer.GetLineWidth(row);
}

// Routine Description:
// - Connects the WriteConsoleInput API call directly into our Driver Message servicing call inside Conhost.exe
// Arguments:
// - events - the input events to be copied into the head of the input
//            buffer for the underlying attached process
// - eventsWritten - on output, the number of events written
// Return Value:
// - true if successful (see DoSrvWriteConsoleInput). false otherwise.
bool ConhostInternalGetSet::PrivateWriteConsoleInputW(std::deque<std::unique_ptr<IInputEvent>>& events,
                                                      size_t& eventsWritten)
{
    eventsWritten = 0;

    return SUCCEEDED(DoSrvPrivateWriteConsoleInputW(_io.GetActiveInputBuffer(),
                                                    events,
                                                    eventsWritten,
                                                    true)); // append
}

// Routine Description:
// - Connects the SetConsoleWindowInfo API call directly into our Driver Message servicing call inside Conhost.exe
// Arguments:
// - absolute - Should the window be moved to an absolute position? If false, the movement is relative to the current pos.
// - window - Info about how to move the viewport
// Return Value:
// - true if successful (see DoSrvSetConsoleWindowInfo). false otherwise.
bool ConhostInternalGetSet::SetConsoleWindowInfo(const bool absolute, const til::inclusive_rect window)
{
    return SUCCEEDED(ServiceLocator::LocateGlobals().api.SetConsoleWindowInfoImpl(_io.GetActiveOutputBuffer(), absolute, window));
}

// Routine Description:
// - Sets the various terminal input modes.
//   SetInputMode is an internal-only "API" call that the vt commands can execute,
//     but it is not represented as a function call on out public API surface.
// Arguments:
// - mode - the input mode to change.
// - enabled - set to true to enable the mode, false to disable it.
// Return Value:
// - true if successful. false otherwise.
bool ConhostInternalGetSet::SetInputMode(const TerminalInput::Mode mode, const bool enabled)
{
    auto& terminalInput = _io.GetActiveInputBuffer()->GetTerminalInput();
    terminalInput.SetInputMode(mode, enabled);

    // If we're a conpty, AND WE'RE IN VT INPUT MODE, always pass input mode requests
    // The VT Input mode check is to work around ssh.exe v7.7, which uses VT
    // output, but not Input.
    // The original comment said, "Once the conpty supports these types of input,
    // this check can be removed. See GH#4911". Unfortunately, time has shown
    // us that SSH 7.7 _also_ requests mouse input and that can have a user interface
    // impact on the actual connected terminal. We can't remove this check,
    // because SSH <=7.7 is out in the wild on all versions of Windows <=2004.
    return !(IsConsolePty() && PrivateIsVtInputEnabled());
}

// Routine Description:
// - Sets the various StateMachine parser modes.
//   SetParserMode is an internal-only "API" call that the vt commands can execute,
//     but it is not represented as a function call on out public API surface.
// Arguments:
// - mode - the parser mode to change.
// - enabled - set to true to enable the mode, false to disable it.
// Return Value:
// - true if successful. false otherwise.
bool ConhostInternalGetSet::SetParserMode(const StateMachine::Mode mode, const bool enabled)
{
    auto& stateMachine = _io.GetActiveOutputBuffer().GetStateMachine();
    stateMachine.SetParserMode(mode, enabled);
    return true;
}

// Routine Description:
// - Retrieves the various StateMachine parser modes.
//   GetParserMode is an internal-only "API" call that the vt commands can execute,
//     but it is not represented as a function call on out public API surface.
// Arguments:
// - mode - the parser mode to query.
// Return Value:
// - true if the mode is enabled. false if disabled.
bool ConhostInternalGetSet::GetParserMode(const Microsoft::Console::VirtualTerminal::StateMachine::Mode mode) const
{
    auto& stateMachine = _io.GetActiveOutputBuffer().GetStateMachine();
    return stateMachine.GetParserMode(mode);
}

// Routine Description:
// - Connects the PrivateSetScreenMode call directly into our Driver Message servicing call inside Conhost.exe
//   PrivateSetScreenMode is an internal-only "API" call that the vt commands can execute,
//     but it is not represented as a function call on our public API surface.
// Arguments:
// - reverseMode - set to true to enable reverse screen mode, false for normal mode.
// Return Value:
// - true if successful (see DoSrvPrivateSetScreenMode). false otherwise.
bool ConhostInternalGetSet::PrivateSetScreenMode(const bool reverseMode)
{
    return NT_SUCCESS(DoSrvPrivateSetScreenMode(reverseMode));
}

// Routine Description:
// - Connects the PrivateSetAutoWrapMode call directly into our Driver Message servicing call inside Conhost.exe
//   PrivateSetAutoWrapMode is an internal-only "API" call that the vt commands can execute,
//     but it is not represented as a function call on out public API surface.
// Arguments:
// - wrapAtEOL - set to true to wrap, false to overwrite the last character.
// Return Value:
// - true if successful (see DoSrvPrivateSetAutoWrapMode). false otherwise.
bool ConhostInternalGetSet::PrivateSetAutoWrapMode(const bool wrapAtEOL)
{
    return NT_SUCCESS(DoSrvPrivateSetAutoWrapMode(wrapAtEOL));
}

// Routine Description:
// - Connects the PrivateShowCursor call directly into our Driver Message servicing call inside Conhost.exe
//   PrivateShowCursor is an internal-only "API" call that the vt commands can execute,
//     but it is not represented as a function call on out public API surface.
// Arguments:
// - show - set to true to make the cursor visible, false to hide.
// Return Value:
// - true if successful (see DoSrvPrivateShowCursor). false otherwise.
bool ConhostInternalGetSet::PrivateShowCursor(const bool show) noexcept
{
    DoSrvPrivateShowCursor(_io.GetActiveOutputBuffer(), show);
    return true;
}

// Routine Description:
// - Connects the PrivateAllowCursorBlinking call directly into our Driver Message servicing call inside Conhost.exe
//   PrivateAllowCursorBlinking is an internal-only "API" call that the vt commands can execute,
//     but it is not represented as a function call on out public API surface.
// Arguments:
// - fEnable - set to true to enable blinking, false to disable
// Return Value:
// - true if successful (see DoSrvPrivateAllowCursorBlinking). false otherwise.
bool ConhostInternalGetSet::PrivateAllowCursorBlinking(const bool fEnable)
{
    DoSrvPrivateAllowCursorBlinking(_io.GetActiveOutputBuffer(), fEnable);

    bool isPty;
    DoSrvIsConsolePty(isPty);
    // If we are connected to a pty, return that we could not handle this
    // so that the VT sequence gets flushed to terminal.
    // Note: we technically don't need to handle it ourselves at all if
    // we are connected to a pty (i.e. we could have just returned false
    // immediately without needing to call DoSrvPrivateAllowCursorBlinking),
    // but we call it anyway for consistency, just in case.
    return !isPty;
}

// Routine Description:
// - Connects the PrivateSetScrollingRegion call directly into our Driver Message servicing call inside Conhost.exe
//   PrivateSetScrollingRegion is an internal-only "API" call that the vt commands can execute,
//     but it is not represented as a function call on out public API surface.
// Arguments:
// - scrollMargins - The bounds of the region to be the scrolling region of the viewport.
// Return Value:
// - true if successful (see DoSrvPrivateSetScrollingRegion). false otherwise.
bool ConhostInternalGetSet::PrivateSetScrollingRegion(const til::inclusive_rect scrollMargins)
{
    return NT_SUCCESS(DoSrvPrivateSetScrollingRegion(_io.GetActiveOutputBuffer(), scrollMargins));
}

// Method Description:
// - Retrieves the current Line Feed/New Line (LNM) mode.
// Arguments:
// - None
// Return Value:
// - true if a line feed also produces a carriage return. false otherwise.
bool ConhostInternalGetSet::PrivateGetLineFeedMode() const
{
    const auto& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
    return gci.IsReturnOnNewlineAutomatic();
}

// Routine Description:
// - Connects the PrivateLineFeed call directly into our Driver Message servicing call inside Conhost.exe
//   PrivateLineFeed is an internal-only "API" call that the vt commands can execute,
//     but it is not represented as a function call on our public API surface.
// Arguments:
// - withReturn - Set to true if a carriage return should be performed as well.
// Return Value:
// - true if successful (see DoSrvPrivateLineFeed). false otherwise.
bool ConhostInternalGetSet::PrivateLineFeed(const bool withReturn)
{
    return NT_SUCCESS(DoSrvPrivateLineFeed(_io.GetActiveOutputBuffer(), withReturn));
}

// Routine Description:
// - Sends a notify message to play the "SystemHand" sound event.
// Return Value:
// - true if successful. false otherwise.
bool ConhostInternalGetSet::PrivateWarningBell()
{
    return _io.GetActiveOutputBuffer().SendNotifyBeep();
}

// Routine Description:
// - Connects the PrivateReverseLineFeed call directly into our Driver Message servicing call inside Conhost.exe
//   PrivateReverseLineFeed is an internal-only "API" call that the vt commands can execute,
//     but it is not represented as a function call on out public API surface.
// Return Value:
// - true if successful (see DoSrvPrivateReverseLineFeed). false otherwise.
bool ConhostInternalGetSet::PrivateReverseLineFeed()
{
    return NT_SUCCESS(DoSrvPrivateReverseLineFeed(_io.GetActiveOutputBuffer()));
}

// Routine Description:
// - Connects the SetConsoleTitleW API call directly into our Driver Message servicing call inside Conhost.exe
// Arguments:
// - title - The null-terminated string to set as the window title
// Return Value:
// - true if successful (see DoSrvSetConsoleTitle). false otherwise.
bool ConhostInternalGetSet::SetConsoleTitleW(std::wstring_view title)
{
    return SUCCEEDED(DoSrvSetConsoleTitleW(title));
}

// Routine Description:
// - Connects the PrivateUseAlternateScreenBuffer call directly into our Driver Message servicing call inside Conhost.exe
//   PrivateUseAlternateScreenBuffer is an internal-only "API" call that the vt commands can execute,
//     but it is not represented as a function call on out public API surface.
// Return Value:
// - true if successful (see DoSrvPrivateUseAlternateScreenBuffer). false otherwise.
bool ConhostInternalGetSet::PrivateUseAlternateScreenBuffer()
{
    return NT_SUCCESS(DoSrvPrivateUseAlternateScreenBuffer(_io.GetActiveOutputBuffer()));
}

// Routine Description:
// - Connects the PrivateUseMainScreenBuffer call directly into our Driver Message servicing call inside Conhost.exe
//   PrivateUseMainScreenBuffer is an internal-only "API" call that the vt commands can execute,
//     but it is not represented as a function call on out public API surface.
// Return Value:
// - true if successful (see DoSrvPrivateUseMainScreenBuffer). false otherwise.
bool ConhostInternalGetSet::PrivateUseMainScreenBuffer()
{
    DoSrvPrivateUseMainScreenBuffer(_io.GetActiveOutputBuffer());
    return true;
}

// Routine Description:
// - Connects the PrivateEraseAll call directly into our Driver Message servicing call inside Conhost.exe
//   PrivateEraseAll is an internal-only "API" call that the vt commands can execute,
//     but it is not represented as a function call on our public API surface.
// Arguments:
// <none>
// Return Value:
// - true if successful (see DoSrvPrivateEraseAll). false otherwise.
bool ConhostInternalGetSet::PrivateEraseAll()
{
    return SUCCEEDED(DoSrvPrivateEraseAll(_io.GetActiveOutputBuffer()));
}

bool ConhostInternalGetSet::PrivateClearBuffer()
{
    return SUCCEEDED(DoSrvPrivateClearBuffer(_io.GetActiveOutputBuffer()));
}

// Method Description:
// - Retrieves the current user default cursor style.
// Arguments:
// - style - Structure to receive cursor style.
// Return Value:
// - true if successful. false otherwise.
bool ConhostInternalGetSet::GetUserDefaultCursorStyle(CursorType& style)
{
    const auto& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
    style = gci.GetCursorType();
    return true;
}

// Routine Description:
// - Connects the SetCursorStyle call directly into our Driver Message servicing call inside Conhost.exe
//   SetCursorStyle is an internal-only "API" call that the vt commands can execute,
//     but it is not represented as a function call on our public API surface.
// Arguments:
// - style: The style of cursor to change the cursor to.
// Return Value:
// - true if successful (see DoSrvSetCursorStyle). false otherwise.
bool ConhostInternalGetSet::SetCursorStyle(const CursorType style)
{
    DoSrvSetCursorStyle(_io.GetActiveOutputBuffer(), style);
    return true;
}

// Routine Description:
// - Connects the PrivatePrependConsoleInput API call directly into our Driver Message servicing call inside Conhost.exe
// Arguments:
// - <none>
// Return Value:
// - true if successful (see DoSrvPrivateRefreshWindow). false otherwise.
bool ConhostInternalGetSet::PrivateRefreshWindow()
{
    DoSrvPrivateRefreshWindow(_io.GetActiveOutputBuffer());
    return true;
}

// Routine Description:
// - Connects the PrivateWriteConsoleControlInput API call directly into our Driver Message servicing call inside Conhost.exe
// Arguments:
// - key - a KeyEvent representing a special type of keypress, typically Ctrl-C
// Return Value:
// - true if successful (see DoSrvPrivateWriteConsoleControlInput). false otherwise.
bool ConhostInternalGetSet::PrivateWriteConsoleControlInput(const KeyEvent key)
{
    return SUCCEEDED(DoSrvPrivateWriteConsoleControlInput(_io.GetActiveInputBuffer(),
                                                          key));
}

// Routine Description:
// - Connects the SetConsoleOutputCP API call directly into our Driver Message servicing call inside Conhost.exe
// Arguments:
// - codepage - the new output codepage of the console.
// Return Value:
// - true if successful (see DoSrvSetConsoleOutputCodePage). false otherwise.
bool ConhostInternalGetSet::SetConsoleOutputCP(const unsigned int codepage)
{
    return SUCCEEDED(DoSrvSetConsoleOutputCodePage(codepage));
}

// Routine Description:
// - Connects the GetConsoleOutputCP API call directly into our Driver Message servicing call inside Conhost.exe
// Arguments:
// - codepage - receives the outputCP of the console.
// Return Value:
// - true if successful (see DoSrvPrivateWriteConsoleControlInput). false otherwise.
bool ConhostInternalGetSet::GetConsoleOutputCP(unsigned int& codepage)
{
    DoSrvGetConsoleOutputCodePage(codepage);
    return true;
}

// Routine Description:
// - Connects the PrivateSuppressResizeRepaint API call directly into our Driver
//      Message servicing call inside Conhost.exe
// Arguments:
// - <none>
// Return Value:
// - true if successful (see DoSrvPrivateSuppressResizeRepaint). false otherwise.
bool ConhostInternalGetSet::PrivateSuppressResizeRepaint()
{
    return SUCCEEDED(DoSrvPrivateSuppressResizeRepaint());
}

// Routine Description:
// - Connects the IsConsolePty call directly into our Driver Message servicing call inside Conhost.exe
// - NOTE: This ONE method behaves differently! The rest of the methods on this
//   interface return true if successful. This one just returns the result.
// Arguments:
// - isPty: receives the bool indicating whether or not we're in pty mode.
// Return Value:
// - true if we're in pty mode.
bool ConhostInternalGetSet::IsConsolePty() const
{
    bool isPty = false;
    DoSrvIsConsolePty(isPty);
    return isPty;
}

bool ConhostInternalGetSet::DeleteLines(const til::CoordType count)
{
    DoSrvPrivateDeleteLines(count);
    return true;
}

bool ConhostInternalGetSet::InsertLines(const til::CoordType count)
{
    DoSrvPrivateInsertLines(count);
    return true;
}

// Method Description:
// - Connects the MoveToBottom call directly into our Driver Message servicing
//      call inside Conhost.exe
// Arguments:
// <none>
// Return Value:
// - true if successful (see DoSrvPrivateMoveToBottom). false otherwise.
bool ConhostInternalGetSet::MoveToBottom() const
{
    DoSrvPrivateMoveToBottom(_io.GetActiveOutputBuffer());
    return true;
}

// Method Description:
// - Retrieves the value in the colortable at the specified index.
// Arguments:
// - tableIndex: the index of the color table to retrieve.
// Return Value:
// - the COLORREF value for the color at that index in the table.
COLORREF ConhostInternalGetSet::GetColorTableEntry(const size_t tableIndex) const noexcept
try
{
    auto& g = ServiceLocator::LocateGlobals();
    auto& gci = g.getConsoleInformation();

    return gci.GetColorTableEntry(tableIndex);
}
catch (...)
{
    return INVALID_COLOR;
}

// Method Description:
// - Updates the value in the colortable at index tableIndex to the new color
//   color. color is a COLORREF, format 0x00BBGGRR.
// Arguments:
// - tableIndex: the index of the color table to update.
// - color: the new COLORREF to use as that color table value.
// Return Value:
// - true if successful. false otherwise.
bool ConhostInternalGetSet::SetColorTableEntry(const size_t tableIndex, const COLORREF color) noexcept
try
{
    auto& g = ServiceLocator::LocateGlobals();
    auto& gci = g.getConsoleInformation();

    gci.SetColorTableEntry(tableIndex, color);

    // If we're setting the default foreground or background colors
    // we need to make sure the index is correctly set as well.
    if (tableIndex == TextColor::DEFAULT_FOREGROUND)
    {
        gci.SetDefaultForegroundIndex(TextColor::DEFAULT_FOREGROUND);
    }
    if (tableIndex == TextColor::DEFAULT_BACKGROUND)
    {
        gci.SetDefaultBackgroundIndex(TextColor::DEFAULT_BACKGROUND);
    }

    // Update the screen colors if we're not a pty
    // No need to force a redraw in pty mode.
    if (g.pRender && !gci.IsInVtIoMode())
    {
        g.pRender->TriggerRedrawAll();
    }

    return true;
}
CATCH_RETURN_FALSE()

// Routine Description:
// - Connects the PrivateFillRegion call directly into our Driver Message servicing
//    call inside Conhost.exe
//   PrivateFillRegion is an internal-only "API" call that the vt commands can execute,
//    but it is not represented as a function call on our public API surface.
// Arguments:
// - screenInfo - Reference to screen buffer info.
// - startPosition - The position to begin filling at.
// - fillLength - The number of characters to fill.
// - fillChar - Character to fill the target region with.
// - standardFillAttrs - If true, fill with the standard erase attributes.
//                       If false, fill with the default attributes.
// Return value:
// - true if successful (see DoSrvPrivateScrollRegion). false otherwise.
bool ConhostInternalGetSet::PrivateFillRegion(const til::point startPosition,
                                              const size_t fillLength,
                                              const wchar_t fillChar,
                                              const bool standardFillAttrs) noexcept
{
    return SUCCEEDED(DoSrvPrivateFillRegion(_io.GetActiveOutputBuffer(),
                                            startPosition,
                                            fillLength,
                                            fillChar,
                                            standardFillAttrs));
}

// Routine Description:
// - Connects the PrivateScrollRegion call directly into our Driver Message servicing
//    call inside Conhost.exe
//   PrivateScrollRegion is an internal-only "API" call that the vt commands can execute,
//    but it is not represented as a function call on our public API surface.
// Arguments:
// - scrollRect - Region to copy/move (source and size).
// - clipRect - Optional clip region to contain buffer change effects.
// - destinationOrigin - Upper left corner of target region.
// - standardFillAttrs - If true, fill with the standard erase attributes.
//                       If false, fill with the default attributes.
// Return value:
// - true if successful (see DoSrvPrivateScrollRegion). false otherwise.
bool ConhostInternalGetSet::PrivateScrollRegion(const til::inclusive_rect scrollRect,
                                                const std::optional<til::inclusive_rect> clipRect,
                                                const til::point destinationOrigin,
                                                const bool standardFillAttrs) noexcept
{
    return SUCCEEDED(DoSrvPrivateScrollRegion(_io.GetActiveOutputBuffer(),
                                              scrollRect,
                                              clipRect,
                                              destinationOrigin,
                                              standardFillAttrs));
}

// Routine Description:
// - Checks if the InputBuffer is willing to accept VT Input directly
//   PrivateIsVtInputEnabled is an internal-only "API" call that the vt commands can execute,
//    but it is not represented as a function call on our public API surface.
// Arguments:
// - <none>
// Return value:
// - true if enabled (see IsInVirtualTerminalInputMode). false otherwise.
bool ConhostInternalGetSet::PrivateIsVtInputEnabled() const
{
    return _io.GetActiveInputBuffer()->IsInVirtualTerminalInputMode();
}

// Method Description:
// - Updates the buffer's current text attributes depending on whether we are
//   starting/ending a hyperlink
// Arguments:
// - The hyperlink URI
// Return Value:
// - true
bool ConhostInternalGetSet::PrivateAddHyperlink(const std::wstring_view uri, const std::wstring_view params) const
{
    DoSrvAddHyperlink(_io.GetActiveOutputBuffer(), uri, params);
    return true;
}

bool ConhostInternalGetSet::PrivateEndHyperlink() const
{
    DoSrvEndHyperlink(_io.GetActiveOutputBuffer());
    return true;
}

// Routine Description:
// - Replaces the active soft font with the given bit pattern.
// Arguments:
// - bitPattern - An array of scanlines representing all the glyphs in the font.
// - cellSize - The cell size for an individual glyph.
// - centeringHint - The horizontal extent that glyphs are offset from center.
// Return Value:
// - true if successful (see DoSrvUpdateSoftFont). false otherwise.
bool ConhostInternalGetSet::PrivateUpdateSoftFont(const gsl::span<const uint16_t> bitPattern,
                                                  const til::size cellSize,
                                                  const size_t centeringHint) noexcept
{
    return SUCCEEDED(DoSrvUpdateSoftFont(bitPattern, cellSize, centeringHint));
}
