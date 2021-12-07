/*++
Copyright (c) Microsoft Corporation
Licensed under the MIT license.

Module Name:
- ApiRoutines.h

Abstract:
- This file defines the interface to respond to all API calls.

Author:
- Michael Niksa (miniksa) 12-Oct-2016

Revision History:
- Adapted from original items in srvinit.cpp, getset.cpp, directio.cpp, stream.cpp
--*/

#pragma once

class ApiRoutines
{
public:
#pragma region ObjectManagement
    /*HRESULT CreateInitialObjects(_Out_ InputBuffer** const ppInputObject,
    _Out_ SCREEN_INFORMATION** const ppOutputObject);
    */

#pragma endregion

#pragma region L1
    void GetConsoleInputCodePageImpl(ULONG& codepage) noexcept;

    void GetConsoleOutputCodePageImpl(ULONG& codepage) noexcept;

    void GetConsoleInputModeImpl(InputBuffer& context,
                                 ULONG& mode) noexcept;

    void GetConsoleOutputModeImpl(SCREEN_INFORMATION& context,
                                  ULONG& mode) noexcept;

    [[nodiscard]] HRESULT SetConsoleInputModeImpl(InputBuffer& context,
                                                  const ULONG mode) noexcept;

    [[nodiscard]] HRESULT SetConsoleOutputModeImpl(SCREEN_INFORMATION& context,
                                                   const ULONG Mode) noexcept;

    [[nodiscard]] HRESULT GetNumberOfConsoleInputEventsImpl(const InputBuffer& context,
                                                            ULONG& events) noexcept;

    [[nodiscard]] HRESULT PeekConsoleInputAImpl(InputBuffer& context,
                                                std::deque<std::unique_ptr<IInputEvent>>& outEvents,
                                                const size_t eventsToRead,
                                                INPUT_READ_HANDLE_DATA& readHandleState,
                                                std::unique_ptr<IWaitRoutine>& waiter) noexcept;

    [[nodiscard]] HRESULT PeekConsoleInputWImpl(InputBuffer& context,
                                                std::deque<std::unique_ptr<IInputEvent>>& outEvents,
                                                const size_t eventsToRead,
                                                INPUT_READ_HANDLE_DATA& readHandleState,
                                                std::unique_ptr<IWaitRoutine>& waiter) noexcept;

    [[nodiscard]] HRESULT ReadConsoleInputAImpl(InputBuffer& context,
                                                std::deque<std::unique_ptr<IInputEvent>>& outEvents,
                                                const size_t eventsToRead,
                                                INPUT_READ_HANDLE_DATA& readHandleState,
                                                std::unique_ptr<IWaitRoutine>& waiter) noexcept;

    [[nodiscard]] HRESULT ReadConsoleInputWImpl(InputBuffer& context,
                                                std::deque<std::unique_ptr<IInputEvent>>& outEvents,
                                                const size_t eventsToRead,
                                                INPUT_READ_HANDLE_DATA& readHandleState,
                                                std::unique_ptr<IWaitRoutine>& waiter) noexcept;

    [[nodiscard]] HRESULT ReadConsoleAImpl(InputBuffer& context,
                                           gsl::span<char> buffer,
                                           size_t& written,
                                           std::unique_ptr<IWaitRoutine>& waiter,
                                           const std::string_view initialData,
                                           const std::wstring_view exeName,
                                           INPUT_READ_HANDLE_DATA& readHandleState,
                                           const HANDLE clientHandle,
                                           const DWORD controlWakeupMask,
                                           DWORD& controlKeyState) noexcept;

    [[nodiscard]] HRESULT ReadConsoleWImpl(InputBuffer& context,
                                           gsl::span<char> buffer,
                                           size_t& written,
                                           std::unique_ptr<IWaitRoutine>& waiter,
                                           const std::string_view initialData,
                                           const std::wstring_view exeName,
                                           INPUT_READ_HANDLE_DATA& readHandleState,
                                           const HANDLE clientHandle,
                                           const DWORD controlWakeupMask,
                                           DWORD& controlKeyState) noexcept;

    [[nodiscard]] HRESULT WriteConsoleAImpl(SCREEN_INFORMATION& context,
                                            const std::string_view buffer,
                                            size_t& read,
                                            bool requiresVtQuirk,
                                            std::unique_ptr<IWaitRoutine>& waiter) noexcept;

    [[nodiscard]] HRESULT WriteConsoleWImpl(SCREEN_INFORMATION& context,
                                            const std::wstring_view buffer,
                                            size_t& read,
                                            bool requiresVtQuirk,
                                            std::unique_ptr<IWaitRoutine>& waiter) noexcept;

#pragma region ThreadCreationInfo
    [[nodiscard]] HRESULT GetConsoleLangIdImpl(LANGID& langId) noexcept;
#pragma endregion

#pragma endregion

#pragma region L2

    [[nodiscard]] HRESULT FillConsoleOutputAttributeImpl(SCREEN_INFORMATION& OutContext,
                                                         const WORD attribute,
                                                         const size_t lengthToWrite,
                                                         const COORD startingCoordinate,
                                                         size_t& cellsModified) noexcept;

    [[nodiscard]] HRESULT FillConsoleOutputCharacterAImpl(SCREEN_INFORMATION& OutContext,
                                                          const char character,
                                                          const size_t lengthToWrite,
                                                          const COORD startingCoordinate,
                                                          size_t& cellsModified) noexcept;

    [[nodiscard]] HRESULT FillConsoleOutputCharacterWImpl(SCREEN_INFORMATION& OutContext,
                                                          const wchar_t character,
                                                          const size_t lengthToWrite,
                                                          const COORD startingCoordinate,
                                                          size_t& cellsModified,
                                                          const bool enablePowershellShim = false) noexcept;

    //// Process based. Restrict in protocol side?
    //HRESULT GenerateConsoleCtrlEventImpl(const ULONG ProcessGroupFilter,
    //                                             const ULONG ControlEvent);

    void SetConsoleActiveScreenBufferImpl(SCREEN_INFORMATION& newContext) noexcept;

    void FlushConsoleInputBuffer(InputBuffer& context) noexcept;

    [[nodiscard]] HRESULT SetConsoleInputCodePageImpl(const ULONG codepage) noexcept;

    [[nodiscard]] HRESULT SetConsoleOutputCodePageImpl(const ULONG codepage) noexcept;

    void GetConsoleCursorInfoImpl(const SCREEN_INFORMATION& context,
                                  ULONG& size,
                                  bool& isVisible) noexcept;

    [[nodiscard]] HRESULT SetConsoleCursorInfoImpl(SCREEN_INFORMATION& context,
                                                   const ULONG size,
                                                   const bool isVisible) noexcept;

    //// driver will pare down for non-Ex method
    void GetConsoleScreenBufferInfoExImpl(const SCREEN_INFORMATION& context,
                                          CONSOLE_SCREEN_BUFFER_INFOEX& data) noexcept;

    [[nodiscard]] HRESULT SetConsoleScreenBufferInfoExImpl(SCREEN_INFORMATION& context,
                                                           const CONSOLE_SCREEN_BUFFER_INFOEX& data) noexcept;

    [[nodiscard]] HRESULT SetConsoleScreenBufferSizeImpl(SCREEN_INFORMATION& context,
                                                         const COORD size) noexcept;

    [[nodiscard]] HRESULT SetConsoleCursorPositionImpl(SCREEN_INFORMATION& context,
                                                       const COORD position) noexcept;

    void GetLargestConsoleWindowSizeImpl(const SCREEN_INFORMATION& context,
                                         COORD& size) noexcept;

    [[nodiscard]] HRESULT ScrollConsoleScreenBufferAImpl(SCREEN_INFORMATION& context,
                                                         const SMALL_RECT& source,
                                                         const COORD target,
                                                         std::optional<SMALL_RECT> clip,
                                                         const char fillCharacter,
                                                         const WORD fillAttribute) noexcept;

    [[nodiscard]] HRESULT ScrollConsoleScreenBufferWImpl(SCREEN_INFORMATION& context,
                                                         const SMALL_RECT& source,
                                                         const COORD target,
                                                         std::optional<SMALL_RECT> clip,
                                                         const wchar_t fillCharacter,
                                                         const WORD fillAttribute,
                                                         const bool enableCmdShim = false) noexcept;

    [[nodiscard]] HRESULT SetConsoleTextAttributeImpl(SCREEN_INFORMATION& context,
                                                      const WORD attribute) noexcept;

    [[nodiscard]] HRESULT SetConsoleWindowInfoImpl(SCREEN_INFORMATION& context,
                                                   const bool isAbsolute,
                                                   const SMALL_RECT& windowRect) noexcept;

    [[nodiscard]] HRESULT ReadConsoleOutputAttributeImpl(const SCREEN_INFORMATION& context,
                                                         const COORD origin,
                                                         gsl::span<WORD> buffer,
                                                         size_t& written) noexcept;

    [[nodiscard]] HRESULT ReadConsoleOutputCharacterAImpl(const SCREEN_INFORMATION& context,
                                                          const COORD origin,
                                                          gsl::span<char> buffer,
                                                          size_t& written) noexcept;

    [[nodiscard]] HRESULT ReadConsoleOutputCharacterWImpl(const SCREEN_INFORMATION& context,
                                                          const COORD origin,
                                                          gsl::span<wchar_t> buffer,
                                                          size_t& written) noexcept;

    [[nodiscard]] HRESULT WriteConsoleInputAImpl(InputBuffer& context,
                                                 const gsl::span<const INPUT_RECORD> buffer,
                                                 size_t& written,
                                                 const bool append) noexcept;

    [[nodiscard]] HRESULT WriteConsoleInputWImpl(InputBuffer& context,
                                                 const gsl::span<const INPUT_RECORD> buffer,
                                                 size_t& written,
                                                 const bool append) noexcept;

    [[nodiscard]] HRESULT WriteConsoleOutputAImpl(SCREEN_INFORMATION& context,
                                                  gsl::span<CHAR_INFO> buffer,
                                                  const Microsoft::Console::Types::Viewport& requestRectangle,
                                                  Microsoft::Console::Types::Viewport& writtenRectangle) noexcept;

    [[nodiscard]] HRESULT WriteConsoleOutputWImpl(SCREEN_INFORMATION& context,
                                                  gsl::span<CHAR_INFO> buffer,
                                                  const Microsoft::Console::Types::Viewport& requestRectangle,
                                                  Microsoft::Console::Types::Viewport& writtenRectangle) noexcept;

    [[nodiscard]] HRESULT WriteConsoleOutputAttributeImpl(SCREEN_INFORMATION& OutContext,
                                                          const gsl::span<const WORD> attrs,
                                                          const COORD target,
                                                          size_t& used) noexcept;

    [[nodiscard]] HRESULT WriteConsoleOutputCharacterAImpl(SCREEN_INFORMATION& OutContext,
                                                           const std::string_view text,
                                                           const COORD target,
                                                           size_t& used) noexcept;

    [[nodiscard]] HRESULT WriteConsoleOutputCharacterWImpl(SCREEN_INFORMATION& OutContext,
                                                           const std::wstring_view text,
                                                           const COORD target,
                                                           size_t& used) noexcept;

    [[nodiscard]] HRESULT ReadConsoleOutputAImpl(const SCREEN_INFORMATION& context,
                                                 gsl::span<CHAR_INFO> buffer,
                                                 const Microsoft::Console::Types::Viewport& sourceRectangle,
                                                 Microsoft::Console::Types::Viewport& readRectangle) noexcept;

    [[nodiscard]] HRESULT ReadConsoleOutputWImpl(const SCREEN_INFORMATION& context,
                                                 gsl::span<CHAR_INFO> buffer,
                                                 const Microsoft::Console::Types::Viewport& sourceRectangle,
                                                 Microsoft::Console::Types::Viewport& readRectangle) noexcept;

    [[nodiscard]] HRESULT GetConsoleTitleAImpl(gsl::span<char> title,
                                               size_t& written,
                                               size_t& needed) noexcept;

    [[nodiscard]] HRESULT GetConsoleTitleWImpl(gsl::span<wchar_t> title,
                                               size_t& written,
                                               size_t& needed) noexcept;

    [[nodiscard]] HRESULT GetConsoleOriginalTitleAImpl(gsl::span<char> title,
                                                       size_t& written,
                                                       size_t& needed) noexcept;

    [[nodiscard]] HRESULT GetConsoleOriginalTitleWImpl(gsl::span<wchar_t> title,
                                                       size_t& written,
                                                       size_t& needed) noexcept;

    [[nodiscard]] HRESULT SetConsoleTitleAImpl(const std::string_view title) noexcept;

    [[nodiscard]] HRESULT SetConsoleTitleWImpl(const std::wstring_view title) noexcept;

#pragma endregion

#pragma region L3
    void GetNumberOfConsoleMouseButtonsImpl(ULONG& buttons) noexcept;

    [[nodiscard]] HRESULT GetConsoleFontSizeImpl(const SCREEN_INFORMATION& context,
                                                 const DWORD index,
                                                 COORD& size) noexcept;

    //// driver will pare down for non-Ex method
    [[nodiscard]] HRESULT GetCurrentConsoleFontExImpl(const SCREEN_INFORMATION& context,
                                                      const bool isForMaximumWindowSize,
                                                      CONSOLE_FONT_INFOEX& consoleFontInfoEx) noexcept;

    [[nodiscard]] HRESULT SetConsoleDisplayModeImpl(SCREEN_INFORMATION& context,
                                                    const ULONG flags,
                                                    COORD& newSize) noexcept;

    void GetConsoleDisplayModeImpl(ULONG& flags) noexcept;

    [[nodiscard]] HRESULT AddConsoleAliasAImpl(const std::string_view source,
                                               const std::string_view target,
                                               const std::string_view exeName) noexcept;

    [[nodiscard]] HRESULT AddConsoleAliasWImpl(const std::wstring_view source,
                                               const std::wstring_view target,
                                               const std::wstring_view exeName) noexcept;

    [[nodiscard]] HRESULT GetConsoleAliasAImpl(const std::string_view source,
                                               gsl::span<char> target,
                                               size_t& written,
                                               const std::string_view exeName) noexcept;

    [[nodiscard]] HRESULT GetConsoleAliasWImpl(const std::wstring_view source,
                                               gsl::span<wchar_t> target,
                                               size_t& written,
                                               const std::wstring_view exeName) noexcept;

    [[nodiscard]] HRESULT GetConsoleAliasesLengthAImpl(const std::string_view exeName,
                                                       size_t& bufferRequired) noexcept;

    [[nodiscard]] HRESULT GetConsoleAliasesLengthWImpl(const std::wstring_view exeName,
                                                       size_t& bufferRequired) noexcept;

    [[nodiscard]] HRESULT GetConsoleAliasExesLengthAImpl(size_t& bufferRequired) noexcept;

    [[nodiscard]] HRESULT GetConsoleAliasExesLengthWImpl(size_t& bufferRequired) noexcept;

    [[nodiscard]] HRESULT GetConsoleAliasesAImpl(const std::string_view exeName,
                                                 gsl::span<char> alias,
                                                 size_t& written) noexcept;

    [[nodiscard]] HRESULT GetConsoleAliasesWImpl(const std::wstring_view exeName,
                                                 gsl::span<wchar_t> alias,
                                                 size_t& written) noexcept;

    [[nodiscard]] HRESULT GetConsoleAliasExesAImpl(gsl::span<char> aliasExes,
                                                   size_t& written) noexcept;

    [[nodiscard]] HRESULT GetConsoleAliasExesWImpl(gsl::span<wchar_t> aliasExes,
                                                   size_t& written) noexcept;

#pragma region CMDext Private API

    [[nodiscard]] HRESULT ExpungeConsoleCommandHistoryAImpl(const std::string_view exeName) noexcept;

    [[nodiscard]] HRESULT ExpungeConsoleCommandHistoryWImpl(const std::wstring_view exeName) noexcept;

    [[nodiscard]] HRESULT SetConsoleNumberOfCommandsAImpl(const std::string_view exeName,
                                                          const size_t numberOfCommands) noexcept;

    [[nodiscard]] HRESULT SetConsoleNumberOfCommandsWImpl(const std::wstring_view exeName,
                                                          const size_t numberOfCommands) noexcept;

    [[nodiscard]] HRESULT GetConsoleCommandHistoryLengthAImpl(const std::string_view exeName,
                                                              size_t& length) noexcept;

    [[nodiscard]] HRESULT GetConsoleCommandHistoryLengthWImpl(const std::wstring_view exeName,
                                                              size_t& length) noexcept;

    [[nodiscard]] HRESULT GetConsoleCommandHistoryAImpl(const std::string_view exeName,
                                                        gsl::span<char> commandHistory,
                                                        size_t& written) noexcept;

    [[nodiscard]] HRESULT GetConsoleCommandHistoryWImpl(const std::wstring_view exeName,
                                                        gsl::span<wchar_t> commandHistory,
                                                        size_t& written) noexcept;

#pragma endregion

    void GetConsoleWindowImpl(HWND& hwnd) noexcept;

    void GetConsoleSelectionInfoImpl(CONSOLE_SELECTION_INFO& consoleSelectionInfo) noexcept;

    void GetConsoleHistoryInfoImpl(CONSOLE_HISTORY_INFO& consoleHistoryInfo) noexcept;

    [[nodiscard]] HRESULT SetConsoleHistoryInfoImpl(const CONSOLE_HISTORY_INFO& consoleHistoryInfo) noexcept;

    [[nodiscard]] HRESULT SetCurrentConsoleFontExImpl(SCREEN_INFORMATION& context,
                                                      const bool isForMaximumWindowSize,
                                                      const CONSOLE_FONT_INFOEX& consoleFontInfoEx) noexcept;

#pragma endregion
};
