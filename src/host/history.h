/*++
Copyright (c) Microsoft Corporation
Licensed under the MIT license.

Module Name:
- history.h

Abstract:
- Encapsulates the cmdline functions and structures specifically related to
        command history functionality.
--*/

#pragma once

class CommandHistory
{
public:
    // CommandHistory Flags
    static constexpr int CLE_ALLOCATED = 0x00000001;
    static constexpr int CLE_RESET = 0x00000002;

    static CommandHistory* s_Allocate(const std::wstring_view appName, const HANDLE processHandle);
    static CommandHistory* s_Find(const HANDLE processHandle);
    static CommandHistory* s_FindByExe(const std::wstring_view appName);
    static void s_ReallocExeToFront(const std::wstring_view appName, const size_t commands);
    static void s_Free(const HANDLE processHandle);
    static void s_ResizeAll(const int commands);
    static int s_CountOfHistories();

    enum class MatchOptions
    {
        None = 0x0,
        ExactMatch = 0x1,
        JustLooking = 0x2
    };

    enum class SearchDirection
    {
        Previous,
        Next
    };

    bool FindMatchingCommand(const std::wstring_view command,
                             const int startingIndex,
                             int& indexFound,
                             const MatchOptions options);
    bool IsAppNameMatch(const std::wstring_view other) const;

    [[nodiscard]] HRESULT Add(const std::wstring_view command,
                              const bool suppressDuplicates);

    [[nodiscard]] HRESULT Retrieve(const SearchDirection searchDirection,
                                   const gsl::span<wchar_t> buffer,
                                   size_t& commandSize);

    [[nodiscard]] HRESULT RetrieveNth(const int index,
                                      const gsl::span<wchar_t> buffer,
                                      size_t& commandSize);

    const std::vector<std::wstring>& GetCommands() const noexcept;
    int GetNumberOfCommands() const noexcept;
    std::wstring_view GetNth(const int index) const noexcept;

    void Realloc(const size_t commands);
    void Empty();

    std::wstring Remove(const int iDel);

    bool AtFirstCommand() const;
    bool AtLastCommand() const;

    std::wstring_view GetLastCommand() const;

    void Swap(const int indexA, const int indexB);

private:
    void _Reset();

    // _Next and _Prev go to the next and prev command
    // _Inc  and _Dec go to the next and prev slots
    // Don't get the two confused - it matters when the cmd history is not full!
    void _Prev(int& ind) const;
    void _Next(int& ind) const;
    void _Dec(int& ind) const;
    void _Inc(int& ind) const;

    std::vector<std::wstring> _commands;
    int _maxCommands;

    std::wstring _appName;
    HANDLE _processHandle;

    static std::list<CommandHistory> s_historyLists;

public:
    DWORD Flags;
    int LastDisplayed;

#ifdef UNIT_TESTING
    static void s_ClearHistoryListStorage();
    friend class HistoryTests;
#endif
};

DEFINE_ENUM_FLAG_OPERATORS(CommandHistory::MatchOptions);
