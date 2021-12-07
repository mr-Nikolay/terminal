// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#pragma once

namespace Microsoft::Console::Interactivity
{
    namespace Win32
    {
        class WindowDpiApi;
    }

    class IInputServices;
    class ISystemConfigurationProvider;
    class IConsoleInputThread;
    class IConsoleControl;

    class InteractivityFactory
    {
    public:
        [[nodiscard]] NTSTATUS CreateConsoleControl(_Inout_ std::unique_ptr<IConsoleControl>& control);
        [[nodiscard]] NTSTATUS CreateConsoleInputThread(_Inout_ std::unique_ptr<IConsoleInputThread>& thread);

        [[nodiscard]] NTSTATUS CreateHighDpiApi(_Inout_ std::unique_ptr<Win32::WindowDpiApi>& api);
        [[nodiscard]] NTSTATUS CreateWindowMetrics(_Inout_ std::unique_ptr<IWindowMetrics>& metrics);
        [[nodiscard]] NTSTATUS CreateAccessibilityNotifier(_Inout_ std::unique_ptr<Win32::AccessibilityNotifier>& notifier);
        [[nodiscard]] NTSTATUS CreateSystemConfigurationProvider(_Inout_ std::unique_ptr<ISystemConfigurationProvider>& provider);
        [[nodiscard]] NTSTATUS CreateInputServices(_Inout_ std::unique_ptr<IInputServices>& services);

        [[nodiscard]] NTSTATUS CreatePseudoWindow(HWND& hwnd);
    };
}
