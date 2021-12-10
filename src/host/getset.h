/*++
Copyright (c) Microsoft Corporation
Licensed under the MIT license.

Module Name:
- getset.h

Abstract:
- This file implements the NT console server console state API.

Author:
- Therese Stowell (ThereseS) 5-Dec-1990

Revision History:
--*/

#pragma once
#include "../inc/conattrs.hpp"
class SCREEN_INFORMATION;

[[nodiscard]] NTSTATUS DoSrvPrivateSetScreenMode(const bool reverseMode);
[[nodiscard]] NTSTATUS DoSrvPrivateSetAutoWrapMode(const bool wrapAtEOL);

void DoSrvPrivateShowCursor(SCREEN_INFORMATION& screenInfo, const bool show) noexcept;
void DoSrvPrivateAllowCursorBlinking(SCREEN_INFORMATION& screenInfo, const bool fEnable);

[[nodiscard]] NTSTATUS DoSrvPrivateSetScrollingRegion(SCREEN_INFORMATION& screenInfo, const til::inclusive_rect scrollMargins);
[[nodiscard]] NTSTATUS DoSrvPrivateLineFeed(SCREEN_INFORMATION& screenInfo, const bool withReturn);
[[nodiscard]] NTSTATUS DoSrvPrivateReverseLineFeed(SCREEN_INFORMATION& screenInfo);

[[nodiscard]] NTSTATUS DoSrvPrivateUseAlternateScreenBuffer(SCREEN_INFORMATION& screenInfo);
void DoSrvPrivateUseMainScreenBuffer(SCREEN_INFORMATION& screenInfo);

[[nodiscard]] HRESULT DoSrvPrivateEraseAll(SCREEN_INFORMATION& screenInfo);
[[nodiscard]] HRESULT DoSrvPrivateClearBuffer(SCREEN_INFORMATION& screenInfo);

void DoSrvSetCursorStyle(SCREEN_INFORMATION& screenInfo,
                         const CursorType cursorType);

void DoSrvAddHyperlink(SCREEN_INFORMATION& screenInfo,
                       const std::wstring_view uri,
                       const std::wstring_view params);

void DoSrvEndHyperlink(SCREEN_INFORMATION& screenInfo);

[[nodiscard]] HRESULT DoSrvUpdateSoftFont(const gsl::span<const uint16_t> bitPattern,
                                          const til::size cellSize,
                                          const size_t centeringHint) noexcept;

void DoSrvPrivateRefreshWindow(const SCREEN_INFORMATION& screenInfo);

[[nodiscard]] HRESULT DoSrvSetConsoleOutputCodePage(const unsigned int codepage);
void DoSrvGetConsoleOutputCodePage(unsigned int& codepage);

[[nodiscard]] NTSTATUS DoSrvPrivateSuppressResizeRepaint();

void DoSrvIsConsolePty(bool& isPty);

void DoSrvPrivateDeleteLines(const til::CoordType count);
void DoSrvPrivateInsertLines(const til::CoordType count);

void DoSrvPrivateMoveToBottom(SCREEN_INFORMATION& screenInfo);

[[nodiscard]] HRESULT DoSrvPrivateFillRegion(SCREEN_INFORMATION& screenInfo,
                                             const til::point startPosition,
                                             const size_t fillLength,
                                             const wchar_t fillChar,
                                             const bool standardFillAttrs) noexcept;

[[nodiscard]] HRESULT DoSrvPrivateScrollRegion(SCREEN_INFORMATION& screenInfo,
                                               const til::inclusive_rect scrollRect,
                                               const std::optional<til::inclusive_rect> clipRect,
                                               const til::point destinationOrigin,
                                               const bool standardFillAttrs) noexcept;
