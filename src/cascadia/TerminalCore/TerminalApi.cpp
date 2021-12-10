// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "Terminal.hpp"
#include "../src/inc/unicode.hpp"

using namespace Microsoft::Terminal::Core;
using namespace Microsoft::Console::Types;
using namespace Microsoft::Console::VirtualTerminal;

// Print puts the text in the buffer and moves the cursor
bool Terminal::PrintString(std::wstring_view stringView) noexcept
try
{
    _WriteBuffer(stringView);
    return true;
}
CATCH_RETURN_FALSE()

bool Terminal::ExecuteChar(wchar_t wch) noexcept
try
{
    _WriteBuffer({ &wch, 1 });
    return true;
}
CATCH_RETURN_FALSE()

TextAttribute Terminal::GetTextAttributes() const noexcept
{
    return _buffer->GetCurrentAttributes();
}

void Terminal::SetTextAttributes(const TextAttribute& attrs) noexcept
{
    _buffer->SetCurrentAttributes(attrs);
}

Viewport Terminal::GetBufferSize() noexcept
{
    return _buffer->GetSize();
}

bool Terminal::SetCursorPosition(til::CoordType x, til::CoordType y) noexcept
try
{
    const auto viewport = _GetMutableViewport();
    const auto viewOrigin = viewport.Origin();
    const auto absoluteX = viewOrigin.x + x;
    const auto absoluteY = viewOrigin.y + y;
    til::point newPos{ absoluteX, absoluteY };
    viewport.Clamp(newPos);
    _buffer->GetCursor().SetPosition(newPos);

    return true;
}
CATCH_RETURN_FALSE()

til::point Terminal::GetCursorPosition() noexcept
{
    const auto absoluteCursorPos = _buffer->GetCursor().GetPosition();
    const auto viewport = _GetMutableViewport();
    const auto viewOrigin = viewport.Origin();
    const auto relativeX = absoluteCursorPos.x - viewOrigin.x;
    const auto relativeY = absoluteCursorPos.y - viewOrigin.y;
    til::point newPos{ relativeX, relativeY };

    // TODO assert that the coord is > (0, 0) && <(view.W, view.H)
    return newPos;
}

// Method Description:
// - Moves the cursor down one line, and possibly also to the leftmost column.
// Arguments:
// - withReturn, set to true if a carriage return should be performed as well.
// Return value:
// - true if succeeded, false otherwise
bool Terminal::CursorLineFeed(const bool withReturn) noexcept
try
{
    auto cursorPos = _buffer->GetCursor().GetPosition();

    // since we explicitly just moved down a row, clear the wrap status on the
    // row we just came from
    _buffer->GetRowByOffset(cursorPos.y).SetWrapForced(false);

    cursorPos.y++;
    if (withReturn)
    {
        cursorPos.x = 0;
    }
    _AdjustCursorPosition(cursorPos);

    return true;
}
CATCH_RETURN_FALSE()

// Method Description:
// - deletes count characters starting from the cursor's current position
// - it moves over the remaining text to 'replace' the deleted text
// - for example, if the buffer looks like this ('|' is the cursor): [abc|def]
// - calling DeleteCharacter(1) will change it to: [abc|ef],
// - i.e. the 'd' gets deleted and the 'ef' gets shifted over 1 space and **retain their previous text attributes**
// Arguments:
// - count, the number of characters to delete
// Return value:
// - true if succeeded, false otherwise
bool Terminal::DeleteCharacter(const til::CoordType count) noexcept
try
{
    const auto cursorPos = _buffer->GetCursor().GetPosition();
    const auto copyToPos = cursorPos;
    const til::point copyFromPos{ cursorPos.x + count, cursorPos.y };
    const auto sourceWidth = _mutableViewport.RightExclusive() - copyFromPos.x;

    // Get a rectangle of the source
    auto source = Viewport::FromDimensions(copyFromPos, sourceWidth, 1);

    // Get a rectangle of the target
    const auto target = Viewport::FromDimensions(copyToPos, source.Dimensions());
    const auto walkDirection = Viewport::DetermineWalkDirection(source, target);

    auto sourcePos = source.GetWalkOrigin(walkDirection);
    auto targetPos = target.GetWalkOrigin(walkDirection);

    // Iterate over the source cell data and copy it over to the target
    do
    {
        const auto data = OutputCell(*(_buffer->GetCellDataAt(sourcePos)));
        _buffer->Write(OutputCellIterator({ &data, 1 }), targetPos);
    } while (source.WalkInBounds(sourcePos, walkDirection) && target.WalkInBounds(targetPos, walkDirection));

    return true;
}
CATCH_RETURN_FALSE()

// Method Description:
// - Inserts count spaces starting from the cursor's current position, moving over the existing text
// - for example, if the buffer looks like this ('|' is the cursor): [abc|def]
// - calling InsertCharacter(1) will change it to: [abc| def],
// - i.e. the 'def' gets shifted over 1 space and **retain their previous text attributes**
// Arguments:
// - count, the number of spaces to insert
// Return value:
// - true if succeeded, false otherwise
bool Terminal::InsertCharacter(const til::CoordType count) noexcept
try
{
    // NOTE: the code below is _extremely_ similar to DeleteCharacter
    // We will want to use this same logic and implement a helper function instead
    // that does the 'move a region from here to there' operation
    // TODO: Github issue #2163

    const auto cursorPos = _buffer->GetCursor().GetPosition();
    const auto copyFromPos = cursorPos;
    const til::point copyToPos{ cursorPos.x + count, cursorPos.y };
    const auto sourceWidth = _mutableViewport.RightExclusive() - copyFromPos.x;

    // Get a rectangle of the source
    auto source = Viewport::FromDimensions(copyFromPos, sourceWidth, 1);

    // Get a rectangle of the target
    const auto target = Viewport::FromDimensions(copyToPos, source.Dimensions());
    const auto walkDirection = Viewport::DetermineWalkDirection(source, target);

    auto sourcePos = source.GetWalkOrigin(walkDirection);
    auto targetPos = target.GetWalkOrigin(walkDirection);

    // Iterate over the source cell data and copy it over to the target
    do
    {
        const auto data = OutputCell(*(_buffer->GetCellDataAt(sourcePos)));
        _buffer->Write(OutputCellIterator({ &data, 1 }), targetPos);
    } while (source.WalkInBounds(sourcePos, walkDirection) && target.WalkInBounds(targetPos, walkDirection));
    const auto eraseIter = OutputCellIterator(UNICODE_SPACE, _buffer->GetCurrentAttributes(), count);
    _buffer->Write(eraseIter, cursorPos);

    return true;
}
CATCH_RETURN_FALSE()

bool Terminal::EraseCharacters(const til::CoordType numChars) noexcept
try
{
    const auto absoluteCursorPos = _buffer->GetCursor().GetPosition();
    const auto viewport = _GetMutableViewport();
    const auto distanceToRight = viewport.RightExclusive() - absoluteCursorPos.x;
    const auto fillLimit = std::min(numChars, distanceToRight);
    const auto eraseIter = OutputCellIterator(UNICODE_SPACE, _buffer->GetCurrentAttributes(), fillLimit);
    _buffer->Write(eraseIter, absoluteCursorPos);
    return true;
}
CATCH_RETURN_FALSE()

// Method description:
// - erases a line of text, either from
// 1. beginning to the cursor's position
// 2. cursor's position to end
// 3. beginning to end
// - depending on the erase type
// Arguments:
// - the erase type
// Return value:
// - true if succeeded, false otherwise
bool Terminal::EraseInLine(const ::Microsoft::Console::VirtualTerminal::DispatchTypes::EraseType eraseType) noexcept
try
{
    const auto cursorPos = _buffer->GetCursor().GetPosition();
    const auto viewport = _GetMutableViewport();
    til::point startPos;
    startPos.y = cursorPos.y;
    // nlength determines the number of spaces we need to write
    DWORD nlength = 0;

    // Determine startPos.x and nlength by the eraseType
    switch (eraseType)
    {
    case DispatchTypes::EraseType::FromBeginning:
        nlength = cursorPos.x - viewport.Left() + 1;
        break;
    case DispatchTypes::EraseType::ToEnd:
        startPos.x = cursorPos.x;
        nlength = viewport.RightExclusive() - startPos.x;
        break;
    case DispatchTypes::EraseType::All:
        startPos.x = viewport.Left();
        nlength = viewport.RightExclusive() - startPos.x;
        break;
    default:
        return false;
    }

    const auto eraseIter = OutputCellIterator(UNICODE_SPACE, _buffer->GetCurrentAttributes(), nlength);

    // Explicitly turn off end-of-line wrap-flag-setting when erasing cells.
    _buffer->Write(eraseIter, startPos, false);
    return true;
}
CATCH_RETURN_FALSE()

// Method description:
// - erases text in the buffer in two ways depending on erase type
// 1. 'erases' all text visible to the user (i.e. the text in the viewport)
// 2. erases all the text in the scrollback
// Arguments:
// - the erase type
// Return Value:
// - true if succeeded, false otherwise
bool Terminal::EraseInDisplay(const DispatchTypes::EraseType eraseType) noexcept
try
{
    // Store the relative cursor position so we can restore it later after we move the viewport
    const auto cursorPos = _buffer->GetCursor().GetPosition();
    auto relativeCursor = cursorPos;
    _mutableViewport.ConvertToOrigin(&relativeCursor);

    // Initialize the new location of the viewport
    // the top and bottom parameters are determined by the eraseType
    til::inclusive_rect newWin;
    newWin.left = _mutableViewport.Left();
    newWin.right = _mutableViewport.RightExclusive();

    if (eraseType == DispatchTypes::EraseType::All)
    {
        // In this case, we simply move the viewport down, effectively pushing whatever text was on the screen into the scrollback
        // and thus 'erasing' the text visible to the user
        const auto coordLastChar = _buffer->GetLastNonSpaceCharacter(_mutableViewport);
        if (coordLastChar.x == 0 && coordLastChar.y == 0)
        {
            // Nothing to clear, just return
            return true;
        }

        auto sNewTop = coordLastChar.y + 1;

        // Increment the circular buffer only if the new location of the viewport would be 'below' the buffer
        const auto delta = (sNewTop + _mutableViewport.Height()) - (_buffer->GetSize().Height());
        for (auto i = 0; i < delta; i++)
        {
            _buffer->IncrementCircularBuffer();
            sNewTop--;
        }

        newWin.top = sNewTop;
        newWin.bottom = sNewTop + _mutableViewport.Height();
    }
    else if (eraseType == DispatchTypes::EraseType::Scrollback)
    {
        // We only want to erase the scrollback, and leave everything else on the screen as it is
        // so we grab the text in the viewport and rotate it up to the top of the buffer
        til::point scrollFromPos{ 0, 0 };
        _mutableViewport.ConvertFromOrigin(&scrollFromPos);
        _buffer->ScrollRows(scrollFromPos.y, _mutableViewport.Height(), -scrollFromPos.y);

        // Since we only did a rotation, the text that was in the scrollback is now _below_ where we are going to move the viewport
        // and we have to make sure we erase that text
        const auto eraseStart = _mutableViewport.Height();
        const auto eraseEnd = _buffer->GetLastNonSpaceCharacter(_mutableViewport).y;
        for (auto i = eraseStart; i <= eraseEnd; i++)
        {
            _buffer->GetRowByOffset(i).Reset(_buffer->GetCurrentAttributes());
        }

        // Reset the scroll offset now because there's nothing for the user to 'scroll' to
        _scrollOffset = 0;

        newWin.top = 0;
        newWin.bottom = _mutableViewport.Height();
    }
    else
    {
        return false;
    }

    // Move the viewport, adjust the scroll bar if needed, and restore the old cursor position
    _mutableViewport = Viewport::FromExclusive(newWin);
    Terminal::_NotifyScrollEvent();
    SetCursorPosition(relativeCursor.x, relativeCursor.y);

    return true;
}
CATCH_RETURN_FALSE()

bool Terminal::WarningBell() noexcept
try
{
    _pfnWarningBell();
    return true;
}
CATCH_RETURN_FALSE()

bool Terminal::SetWindowTitle(std::wstring_view title) noexcept
try
{
    if (!_suppressApplicationTitle)
    {
        _title.emplace(title);
        _pfnTitleChanged(_title.value());
    }
    return true;
}
CATCH_RETURN_FALSE()

// Method Description:
// - Retrieves the value in the colortable at the specified index.
// Arguments:
// - tableIndex: the index of the color table to retrieve.
// Return Value:
// - the COLORREF value for the color at that index in the table.
COLORREF Terminal::GetColorTableEntry(const size_t tableIndex) const noexcept
try
{
    return _colorTable.at(tableIndex);
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
// - true iff we successfully updated the color table entry.
bool Terminal::SetColorTableEntry(const size_t tableIndex, const COLORREF color) noexcept
try
{
    _colorTable.at(tableIndex) = color;

    if (tableIndex == TextColor::DEFAULT_BACKGROUND)
    {
        _pfnBackgroundColorChanged(color);
    }

    // Repaint everything - the colors might have changed
    _buffer->GetRenderTarget().TriggerRedrawAll();
    return true;
}
CATCH_RETURN_FALSE()

// Method Description:
// - Sets the cursor style to the given style.
// Arguments:
// - cursorStyle: the style to be set for the cursor
// Return Value:
// - true iff we successfully set the cursor style
bool Terminal::SetCursorStyle(const DispatchTypes::CursorStyle cursorStyle) noexcept
{
    CursorType finalCursorType = CursorType::Legacy;
    bool shouldBlink = false;

    switch (cursorStyle)
    {
    case DispatchTypes::CursorStyle::UserDefault:
        finalCursorType = _defaultCursorShape;
        shouldBlink = true;
        break;
    case DispatchTypes::CursorStyle::BlinkingBlock:
        finalCursorType = CursorType::FullBox;
        shouldBlink = true;
        break;
    case DispatchTypes::CursorStyle::SteadyBlock:
        finalCursorType = CursorType::FullBox;
        shouldBlink = false;
        break;
    case DispatchTypes::CursorStyle::BlinkingUnderline:
        finalCursorType = CursorType::Underscore;
        shouldBlink = true;
        break;
    case DispatchTypes::CursorStyle::SteadyUnderline:
        finalCursorType = CursorType::Underscore;
        shouldBlink = false;
        break;
    case DispatchTypes::CursorStyle::BlinkingBar:
        finalCursorType = CursorType::VerticalBar;
        shouldBlink = true;
        break;
    case DispatchTypes::CursorStyle::SteadyBar:
        finalCursorType = CursorType::VerticalBar;
        shouldBlink = false;
        break;

    default:
        // Invalid argument should be ignored.
        return true;
    }

    _buffer->GetCursor().SetType(finalCursorType);
    _buffer->GetCursor().SetBlinkingAllowed(shouldBlink);

    return true;
}

bool Terminal::SetInputMode(const TerminalInput::Mode mode, const bool enabled) noexcept
try
{
    _terminalInput->SetInputMode(mode, enabled);
    return true;
}
CATCH_RETURN_FALSE()

bool Terminal::SetScreenMode(const bool reverseMode) noexcept
try
{
    _screenReversed = reverseMode;

    // Repaint everything - the colors will have changed
    _buffer->GetRenderTarget().TriggerRedrawAll();
    return true;
}
CATCH_RETURN_FALSE()

bool Terminal::EnableXtermBracketedPasteMode(const bool enabled) noexcept
{
    _bracketedPasteMode = enabled;
    return true;
}

bool Terminal::IsXtermBracketedPasteModeEnabled() const noexcept
{
    return _bracketedPasteMode;
}

bool Terminal::IsVtInputEnabled() const noexcept
{
    // We should never be getting this call in Terminal.
    FAIL_FAST();
}

bool Terminal::SetCursorVisibility(const bool visible) noexcept
{
    _buffer->GetCursor().SetIsVisible(visible);
    return true;
}

bool Terminal::EnableCursorBlinking(const bool enable) noexcept
{
    _buffer->GetCursor().SetBlinkingAllowed(enable);

    // GH#2642 - From what we've gathered from other terminals, when blinking is
    // disabled, the cursor should remain On always, and have the visibility
    // controlled by the IsVisible property. So when you do a printf "\e[?12l"
    // to disable blinking, the cursor stays stuck On. At this point, only the
    // cursor visibility property controls whether the user can see it or not.
    // (Yes, the cursor can be On and NOT Visible)
    _buffer->GetCursor().SetIsOn(true);
    return true;
}

bool Terminal::CopyToClipboard(std::wstring_view content) noexcept
try
{
    _pfnCopyToClipboard(content);

    return true;
}
CATCH_RETURN_FALSE()

// Method Description:
// - Updates the buffer's current text attributes to start a hyperlink
// Arguments:
// - The hyperlink URI
// - The customID provided (if there was one)
// Return Value:
// - true
bool Terminal::AddHyperlink(std::wstring_view uri, std::wstring_view params) noexcept
{
    auto attr = _buffer->GetCurrentAttributes();
    const auto id = _buffer->GetHyperlinkId(uri, params);
    attr.SetHyperlinkId(id);
    _buffer->SetCurrentAttributes(attr);
    _buffer->AddHyperlinkToMap(uri, id);
    return true;
}

// Method Description:
// - Updates the buffer's current text attributes to end a hyperlink
// Return Value:
// - true
bool Terminal::EndHyperlink() noexcept
{
    auto attr = _buffer->GetCurrentAttributes();
    attr.SetHyperlinkId(0);
    _buffer->SetCurrentAttributes(attr);
    return true;
}

// Method Description:
// - Updates the taskbar progress indicator
// Arguments:
// - state: indicates the progress state
// - progress: indicates the progress value
// Return Value:
// - true
bool Terminal::SetTaskbarProgress(const ::Microsoft::Console::VirtualTerminal::DispatchTypes::TaskbarState state, const size_t progress) noexcept
{
    _taskbarState = static_cast<size_t>(state);

    switch (state)
    {
    case DispatchTypes::TaskbarState::Clear:
        // Always set progress to 0 in this case
        _taskbarProgress = 0;
        break;
    case DispatchTypes::TaskbarState::Set:
        // Always set progress to the value given in this case
        _taskbarProgress = progress;
        break;
    case DispatchTypes::TaskbarState::Indeterminate:
        // Leave the progress value unchanged in this case
        break;
    case DispatchTypes::TaskbarState::Error:
    case DispatchTypes::TaskbarState::Paused:
        // In these 2 cases, if the given progress value is 0, then
        // leave the progress value unchanged, unless the current progress
        // value is 0, in which case set it to a 'minimum' value (10 in our case);
        // if the given progress value is greater than 0, then set the progress value
        if (progress == 0)
        {
            if (_taskbarProgress == 0)
            {
                _taskbarProgress = TaskbarMinProgress;
            }
        }
        else
        {
            _taskbarProgress = progress;
        }
        break;
    }

    if (_pfnTaskbarProgressChanged)
    {
        _pfnTaskbarProgressChanged();
    }
    return true;
}

bool Terminal::SetWorkingDirectory(std::wstring_view uri) noexcept
{
    _workingDirectory = uri;
    return true;
}

std::wstring_view Terminal::GetWorkingDirectory() noexcept
{
    return _workingDirectory;
}

// Method Description:
// - Saves the current text attributes to an internal stack.
// Arguments:
// - options, cOptions: if present, specify which portions of the current text attributes
//   should be saved. Only a small subset of GraphicsOptions are actually supported;
//   others are ignored. If no options are specified, all attributes are stored.
// Return Value:
// - true
bool Terminal::PushGraphicsRendition(const VTParameters options) noexcept
{
    _sgrStack.Push(_buffer->GetCurrentAttributes(), options);
    return true;
}

// Method Description:
// - Restores text attributes from the internal stack. If only portions of text attributes
//   were saved, combines those with the current attributes.
// Arguments:
// - <none>
// Return Value:
// - true
bool Terminal::PopGraphicsRendition() noexcept
{
    const TextAttribute current = _buffer->GetCurrentAttributes();
    _buffer->SetCurrentAttributes(_sgrStack.Pop(current));
    return true;
}
