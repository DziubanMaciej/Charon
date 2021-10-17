#pragma once

#include "charon/user_interface/daemon_user_interface.h"
#include "charon/user_interface/windows/tray_icon.h"
#include "charon/user_interface/windows/window_class.h"

class DaemonUserInterfaceWindows : public DaemonUserInterface {
public:
    DaemonUserInterfaceWindows(Charon &charon);

    void run() override;

private:
    // Wwindow creation
    void createWindow();
    void destroyWindow();

    // Accessing instance of this class through HWND
    static void setUserDataForHandle(HWND windowHandle, DaemonUserInterfaceWindows *windowProcImpl);
    static DaemonUserInterfaceWindows *getUserDataFromHandle(HWND windowHandle);
    static DaemonUserInterfaceWindows *getUserData(HWND windowHandle, UINT message, LPARAM lParam);

    // Window callbacks
    static LRESULT CALLBACK windowProcNative(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam); // called by win32
    LRESULT windowProcImpl(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);                   // actual procedure, called by windowProcNative

    void openConfigFile();

    constexpr static UINT WMAPP_TrayIconCallback = WM_APP + 1;

    // Data
    const HINSTANCE instanceHandle;
    const WindowClass windowClass;
    HWND windowHandle{};
    std::unique_ptr<TrayIcon> trayIcon{};
};
