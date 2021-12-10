// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "precomp.h"

#include "BgfxEngine.hpp"

#include "ConIoSrvComm.hpp"
#include "../inc/ServiceLocator.hpp"

#pragma hdrstop

//
// Default non-bright white.
//

#define DEFAULT_COLOR_ATTRIBUTE (0xC)

using namespace Microsoft::Console::Render;
using namespace Microsoft::Console::Interactivity;
using namespace Microsoft::Console::Interactivity::OneCore;

BgfxEngine::BgfxEngine(PVOID SharedViewBase, LONG DisplayHeight, LONG DisplayWidth, LONG FontWidth, LONG FontHeight) :
    RenderEngineBase(),
    _sharedViewBase((ULONG_PTR)SharedViewBase),
    _displayHeight(DisplayHeight),
    _displayWidth(DisplayWidth),
    _currentLegacyColorAttribute(DEFAULT_COLOR_ATTRIBUTE)
{
    _runLength = sizeof(CD_IO_CHARACTER) * DisplayWidth;

    _fontSize.x = FontWidth > SHORT_MAX ? SHORT_MAX : FontWidth;
    _fontSize.y = FontHeight > SHORT_MAX ? SHORT_MAX : FontHeight;
}

[[nodiscard]] HRESULT BgfxEngine::Invalidate(const til::inclusive_rect /*psrRegion*/) noexcept
{
    return S_OK;
}

[[nodiscard]] HRESULT BgfxEngine::InvalidateCursor(const til::inclusive_rect /*psrRegion*/) noexcept
{
    return S_OK;
}

[[nodiscard]] HRESULT BgfxEngine::InvalidateSystem(const til::rect& /*prcDirtyClient*/) noexcept
{
    return S_OK;
}

[[nodiscard]] HRESULT BgfxEngine::InvalidateSelection(const std::vector<til::inclusive_rect>& /*rectangles*/) noexcept
{
    return S_OK;
}

[[nodiscard]] HRESULT BgfxEngine::InvalidateScroll(til::point /*pcoordDelta*/) noexcept
{
    return S_OK;
}

[[nodiscard]] HRESULT BgfxEngine::InvalidateAll() noexcept
{
    return S_OK;
}

[[nodiscard]] HRESULT BgfxEngine::InvalidateCircling(_Out_ bool* const pForcePaint) noexcept
{
    *pForcePaint = false;
    return S_FALSE;
}

[[nodiscard]] HRESULT BgfxEngine::PrepareForTeardown(_Out_ bool* const pForcePaint) noexcept
{
    *pForcePaint = false;
    return S_FALSE;
}

[[nodiscard]] HRESULT BgfxEngine::StartPaint() noexcept
{
    return S_OK;
}

[[nodiscard]] HRESULT BgfxEngine::EndPaint() noexcept
{
    NTSTATUS Status;

    PVOID OldRunBase;
    PVOID NewRunBase;

    Status = ServiceLocator::LocateInputServices<ConIoSrvComm>()->RequestUpdateDisplay(0);

    if (NT_SUCCESS(Status))
    {
        for (auto i = 0; i < _displayHeight; i++)
        {
            OldRunBase = (PVOID)(_sharedViewBase + (i * 2 * _runLength));
            NewRunBase = (PVOID)(_sharedViewBase + (i * 2 * _runLength) + _runLength);
            memcpy_s(OldRunBase, _runLength, NewRunBase, _runLength);
        }
    }

    return HRESULT_FROM_NT(Status);
}

// Routine Description:
// - Used to perform longer running presentation steps outside the lock so the other threads can continue.
// - Not currently used by BgfxEngine.
// Arguments:
// - <none>
// Return Value:
// - S_FALSE since we do nothing.
[[nodiscard]] HRESULT BgfxEngine::Present() noexcept
{
    return S_FALSE;
}

[[nodiscard]] HRESULT BgfxEngine::ScrollFrame() noexcept
{
    return S_OK;
}

[[nodiscard]] HRESULT BgfxEngine::PaintBackground() noexcept
{
    PVOID OldRunBase;
    PVOID NewRunBase;

    PCD_IO_CHARACTER OldRun;
    PCD_IO_CHARACTER NewRun;

    for (auto i = 0; i < _displayHeight; i++)
    {
        OldRunBase = (PVOID)(_sharedViewBase + (i * 2 * _runLength));
        NewRunBase = (PVOID)(_sharedViewBase + (i * 2 * _runLength) + _runLength);

        OldRun = (PCD_IO_CHARACTER)OldRunBase;
        NewRun = (PCD_IO_CHARACTER)NewRunBase;

        for (auto j = 0; j < _displayWidth; j++)
        {
            NewRun[j].Character = L' ';
            NewRun[j].Attribute = 0;
        }
    }

    return S_OK;
}

[[nodiscard]] HRESULT BgfxEngine::PaintBufferLine(const gsl::span<const Cluster> clusters,
                                                  const til::point coord,
                                                  const bool /*trimLeft*/,
                                                  const bool /*lineWrapped*/) noexcept
{
    try
    {
        PVOID NewRunBase = (PVOID)(_sharedViewBase + (coord.y * 2 * _runLength) + _runLength);
        PCD_IO_CHARACTER NewRun = (PCD_IO_CHARACTER)NewRunBase;

        for (size_t i = 0; i < clusters.size() && i < (size_t)_displayWidth; i++)
        {
            NewRun[coord.x + i].Character = til::at(clusters, i).GetTextAsSingle();
            NewRun[coord.x + i].Attribute = _currentLegacyColorAttribute;
        }

        return S_OK;
    }
    CATCH_RETURN();
}

[[nodiscard]] HRESULT BgfxEngine::PaintBufferGridLines(GridLineSet const /*lines*/,
                                                       COLORREF const /*color*/,
                                                       size_t const /*cchLine*/,
                                                       til::point /*coordTarget*/) noexcept
{
    return S_OK;
}

[[nodiscard]] HRESULT BgfxEngine::PaintSelection(const til::rect& /*rect*/) noexcept
{
    return S_OK;
}

[[nodiscard]] HRESULT BgfxEngine::PaintCursor(const CursorOptions& options) noexcept
{
    // TODO: MSFT: 11448021 - Modify BGFX to support rendering full-width
    // characters and a full-width cursor.
    CD_IO_CURSOR_INFORMATION CursorInfo;
    CursorInfo.Row = options.coordCursor.y;
    CursorInfo.Column = options.coordCursor.x;
    CursorInfo.Height = options.ulCursorHeightPercent;
    CursorInfo.IsVisible = TRUE;

    NTSTATUS Status = ServiceLocator::LocateInputServices<ConIoSrvComm>()->RequestSetCursor(&CursorInfo);

    return HRESULT_FROM_NT(Status);
}

[[nodiscard]] HRESULT BgfxEngine::UpdateDrawingBrushes(const TextAttribute& textAttributes,
                                                       const gsl::not_null<IRenderData*> /*pData*/,
                                                       const bool /*usingSoftFont*/,
                                                       bool const /*isSettingDefaultBrushes*/) noexcept
{
    _currentLegacyColorAttribute = textAttributes.GetLegacyAttributes();

    return S_OK;
}

[[nodiscard]] HRESULT BgfxEngine::UpdateFont(const FontInfoDesired& /*pfiFontInfoDesired*/, FontInfo& /*pfiFontInfo*/) noexcept
{
    return S_OK;
}

[[nodiscard]] HRESULT BgfxEngine::UpdateDpi(int const /*iDpi*/) noexcept
{
    return S_OK;
}

// Method Description:
// - This method will update our internal reference for how big the viewport is.
//      Does nothing for BGFX.
// Arguments:
// - srNewViewport - The bounds of the new viewport.
// Return Value:
// - HRESULT S_OK
[[nodiscard]] HRESULT BgfxEngine::UpdateViewport(const til::inclusive_rect /*srNewViewport*/) noexcept
{
    return S_OK;
}

[[nodiscard]] HRESULT BgfxEngine::GetProposedFont(const FontInfoDesired& /*pfiFontInfoDesired*/, FontInfo& /*pfiFontInfo*/, int const /*iDpi*/) noexcept
{
    return S_OK;
}

[[nodiscard]] HRESULT BgfxEngine::GetDirtyArea(gsl::span<const til::rect>& area) noexcept
{
    til::inclusive_rect r;
    r.bottom = _displayHeight > 0 ? (_displayHeight - 1) : 0;
    r.top = 0;
    r.left = 0;
    r.right = _displayWidth > 0 ? (_displayWidth - 1) : 0;

    _dirtyArea = r;

    area = { &_dirtyArea,
             1 };

    return S_OK;
}

[[nodiscard]] HRESULT BgfxEngine::GetFontSize(_Out_ til::size& pFontSize) noexcept
{
    *pFontSize = _fontSize;
    return S_OK;
}

[[nodiscard]] HRESULT BgfxEngine::IsGlyphWideByFont(const std::wstring_view /*glyph*/, _Out_ bool* const pResult) noexcept
{
    *pResult = false;
    return S_OK;
}

// Method Description:
// - Updates the window's title string.
//      Does nothing for BGFX.
// Arguments:
// - newTitle: the new string to use for the title of the window
// Return Value:
// - S_OK
[[nodiscard]] HRESULT BgfxEngine::_DoUpdateTitle(_In_ const std::wstring_view /*newTitle*/) noexcept
{
    return S_OK;
}
