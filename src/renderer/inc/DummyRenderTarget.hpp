/*++
Copyright (c) Microsoft Corporation
Licensed under the MIT license.

Module Name:
- DummyRenderTarget.hpp

Abstract:
- Provides an empty implementation of the IRenderTarget interface.
    This is needed for some tests, where certain objects need a reference to a
    IRenderTarget

Author(s):
- Mike Griese (migrie) Nov 2018
--*/

#pragma once
#include "IRenderTarget.hpp"

class DummyRenderTarget final : public Microsoft::Console::Render::IRenderTarget
{
public:
    DummyRenderTarget() {}
    void TriggerRedraw(const Microsoft::Console::Types::Viewport& /*region*/) override {}
    void TriggerRedraw(const til::point* const /*pcoord*/) override {}
    void TriggerRedrawCursor(const til::point* const /*pcoord*/) override {}
    void TriggerRedrawAll() override {}
    void TriggerTeardown() noexcept override {}
    void TriggerSelection() override {}
    void TriggerScroll() override {}
    void TriggerScroll(const til::point* const /*pcoordDelta*/) override {}
    void TriggerCircling() override {}
    void TriggerTitleChange() override {}
};
