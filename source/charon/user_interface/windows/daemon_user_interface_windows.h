#pragma once

#include "charon/user_interface/daemon_user_interface.h"
#include "charon/user_interface/windows/tray_icon.h"
#include "charon/user_interface/windows/window.h"
#include "charon/user_interface/windows/window_class.h"

class DaemonUserInterfaceWindows : public DaemonUserInterface {
public:
    DaemonUserInterfaceWindows(Charon &charon);
    void run() override;

    constexpr static UINT WMAPP_TrayIconCallback = WM_APP + 1;
    constexpr static UINT WMAPP_Init = WM_APP + 2;
    LRESULT windowProcImpl(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam); // actual procedure, called by windowProcNative
private:
    void openTextFile(const fs::path &path);

    const HINSTANCE instanceHandle;
    const WindowClass windowClass;
    Window window;
    std::unique_ptr<TrayIcon> trayIcon{};
};
