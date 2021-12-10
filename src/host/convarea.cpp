// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "precomp.h"

#include "_output.h"

#include "../interactivity/inc/ServiceLocator.hpp"

#pragma hdrstop

using namespace Microsoft::Console::Types;
using Microsoft::Console::Interactivity::ServiceLocator;

bool IsValidRect(_In_ til::rect* const Rect)
{
    return (Rect->right >= Rect->left && Rect->bottom >= Rect->top);
}

void WriteConvRegionToScreen(const SCREEN_INFORMATION& ScreenInfo,
                             const Viewport& convRegion)
{
    CONSOLE_INFORMATION& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
    if (!ScreenInfo.IsActiveScreenBuffer())
    {
        return;
    }

    ConsoleImeInfo* const pIme = &gci.ConsoleIme;

    for (unsigned int i = 0; i < pIme->ConvAreaCompStr.size(); ++i)
    {
        const auto& ConvAreaInfo = pIme->ConvAreaCompStr[i];

        if (!ConvAreaInfo.IsHidden())
        {
            const auto currentViewport = ScreenInfo.GetViewport().ToInclusive();
            const auto areaInfo = ConvAreaInfo.GetAreaBufferInfo();

            // Do clipping region
            til::rect Region;
            Region.left = currentViewport.left + areaInfo.rcViewCaWindow.left + areaInfo.coordConView.x;
            Region.right = Region.left + (areaInfo.rcViewCaWindow.right - areaInfo.rcViewCaWindow.left);
            Region.top = currentViewport.top + areaInfo.rcViewCaWindow.top + areaInfo.coordConView.y;
            Region.bottom = Region.top + (areaInfo.rcViewCaWindow.bottom - areaInfo.rcViewCaWindow.top);

            til::rect ClippedRegion;
            ClippedRegion.left = std::max(Region.left, currentViewport.left);
            ClippedRegion.top = std::max(Region.top, currentViewport.top);
            ClippedRegion.right = std::min(Region.right, currentViewport.right);
            ClippedRegion.bottom = std::min(Region.bottom, currentViewport.bottom);

            if (IsValidRect(&ClippedRegion))
            {
                Region = ClippedRegion;
                ClippedRegion.left = std::max(Region.left, convRegion.Left());
                ClippedRegion.top = std::max(Region.top, convRegion.Top());
                ClippedRegion.right = std::min(Region.right, convRegion.RightInclusive());
                ClippedRegion.bottom = std::min(Region.bottom, convRegion.BottomInclusive());
                if (IsValidRect(&ClippedRegion))
                {
                    // if we have a renderer, we need to update.
                    // we've already confirmed (above with an early return) that we're on conversion areas that are a part of the active (visible/rendered) screen
                    // so send invalidates to those regions such that we're queried for data on the next frame and repainted.
                    if (ServiceLocator::LocateGlobals().pRender != nullptr)
                    {
                        // convert inclusive rectangle to exclusive rectangle
                        auto srExclusive = ClippedRegion;
                        srExclusive.right++;
                        srExclusive.bottom++;

                        ServiceLocator::LocateGlobals().pRender->TriggerRedraw(Viewport::FromExclusive(srExclusive));
                    }
                }
            }
        }
    }
}

[[nodiscard]] HRESULT ConsoleImeResizeCompStrView()
{
    try
    {
        CONSOLE_INFORMATION& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
        ConsoleImeInfo* const pIme = &gci.ConsoleIme;
        pIme->RedrawCompMessage();
    }
    CATCH_RETURN();

    return S_OK;
}

[[nodiscard]] HRESULT ConsoleImeResizeCompStrScreenBuffer(const til::size coordNewScreenSize)
{
    CONSOLE_INFORMATION& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
    ConsoleImeInfo* const pIme = &gci.ConsoleIme;

    return pIme->ResizeAllAreas(coordNewScreenSize);
}

[[nodiscard]] HRESULT ImeStartComposition()
{
    auto& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
    gci.LockConsole();
    auto unlock = wil::scope_exit([&] { gci.UnlockConsole(); });

    // MSFT:29219348 Some IME implementations do not produce composition strings, and
    // their users have come to rely on the cursor that conhost traditionally left on
    // until a composition string showed up.
    // One such IME is WNWB's "Universal Wubi input method" from wnwb.com (v. 10+).
    // We shouldn't hide the cursor here so as to not break those IMEs.

    gci.pInputBuffer->fInComposition = true;
    return S_OK;
}

[[nodiscard]] HRESULT ImeEndComposition()
{
    auto& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
    gci.LockConsole();
    auto unlock = wil::scope_exit([&] { gci.UnlockConsole(); });

    ConsoleImeInfo* const pIme = &gci.ConsoleIme;
    pIme->RestoreCursorVisibility();

    gci.pInputBuffer->fInComposition = false;
    return S_OK;
}

[[nodiscard]] HRESULT ImeComposeData(std::wstring_view text,
                                     gsl::span<const BYTE> attributes,
                                     gsl::span<const WORD> colorArray)
{
    try
    {
        CONSOLE_INFORMATION& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
        gci.LockConsole();
        auto unlock = wil::scope_exit([&] { gci.UnlockConsole(); });

        ConsoleImeInfo* const pIme = &gci.ConsoleIme;
        pIme->WriteCompMessage(text, attributes, colorArray);
    }
    CATCH_RETURN();
    return S_OK;
}

[[nodiscard]] HRESULT ImeClearComposeData()
{
    try
    {
        CONSOLE_INFORMATION& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
        gci.LockConsole();
        auto unlock = wil::scope_exit([&] { gci.UnlockConsole(); });

        ConsoleImeInfo* const pIme = &gci.ConsoleIme;
        pIme->ClearAllAreas();
    }
    CATCH_RETURN();
    return S_OK;
}

[[nodiscard]] HRESULT ImeComposeResult(std::wstring_view text)
{
    try
    {
        CONSOLE_INFORMATION& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
        gci.LockConsole();
        auto unlock = wil::scope_exit([&] { gci.UnlockConsole(); });

        ConsoleImeInfo* const pIme = &gci.ConsoleIme;
        pIme->WriteResultMessage(text);
    }
    CATCH_RETURN();
    return S_OK;
}
