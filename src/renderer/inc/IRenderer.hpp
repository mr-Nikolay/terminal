/*++
Copyright (c) Microsoft Corporation
Licensed under the MIT license.

Module Name:
- IRenderer.hpp

Abstract:
- This serves as the entry point for console rendering activities.

Author(s):
- Michael Niksa (MiNiksa) 17-Nov-2015
--*/

#pragma once

#include "FontInfoDesired.hpp"
#include "IRenderEngine.hpp"
#include "IRenderTarget.hpp"
#include "../types/inc/viewport.hpp"

namespace Microsoft::Console::Render
{
    class IRenderer : public IRenderTarget
    {
    public:
        ~IRenderer() = 0;
        IRenderer(const IRenderer&) = default;
        IRenderer(IRenderer&&) = default;
        IRenderer& operator=(const IRenderer&) = default;
        IRenderer& operator=(IRenderer&&) = default;

        [[nodiscard]] virtual HRESULT PaintFrame() = 0;

        virtual void TriggerSystemRedraw(const til::rect& prcDirtyClient) = 0;

        virtual void TriggerRedraw(const Microsoft::Console::Types::Viewport& region) = 0;
        virtual void TriggerRedraw(const til::point* const pcoord) = 0;
        virtual void TriggerRedrawCursor(const til::point* const pcoord) = 0;

        virtual void TriggerRedrawAll() = 0;
        virtual void TriggerTeardown() noexcept = 0;

        virtual void TriggerSelection() = 0;
        virtual void TriggerScroll() = 0;
        virtual void TriggerScroll(const til::point* const pcoordDelta) = 0;
        virtual void TriggerCircling() = 0;
        virtual void TriggerTitleChange() = 0;
        virtual void TriggerFontChange(const int iDpi,
                                       const FontInfoDesired& FontInfoDesired,
                                       _Out_ FontInfo& FontInfo) = 0;

        virtual void UpdateSoftFont(const gsl::span<const uint16_t> bitPattern,
                                    const til::size cellSize,
                                    const size_t centeringHint) = 0;

        [[nodiscard]] virtual HRESULT GetProposedFont(const int iDpi,
                                                      const FontInfoDesired& FontInfoDesired,
                                                      _Out_ FontInfo& FontInfo) = 0;

        virtual bool IsGlyphWideByFont(const std::wstring_view glyph) = 0;

        virtual void EnablePainting() = 0;
        virtual void WaitForPaintCompletionAndDisable(const DWORD dwTimeoutMs) = 0;
        virtual void WaitUntilCanRender() = 0;

        virtual void AddRenderEngine(_In_ IRenderEngine* const pEngine) = 0;

    protected:
        IRenderer() = default;
    };

    inline Microsoft::Console::Render::IRenderer::~IRenderer() {}

}
