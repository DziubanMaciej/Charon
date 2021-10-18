
#include "charon/user_interface/windows/daemon_user_interface_windows.h"
#include "charon/util/error.h"

#include <shellapi.h>

std::unique_ptr<DaemonUserInterface> DaemonUserInterface::create(Charon &charon) {
    return std::unique_ptr<DaemonUserInterface>(new DaemonUserInterfaceWindows(charon));
}

DaemonUserInterfaceWindows::DaemonUserInterfaceWindows(Charon &charon)
    : DaemonUserInterface(charon),
      instanceHandle(GetModuleHandle(NULL)),
      windowClass(instanceHandle, Window::windowProcNative<DaemonUserInterfaceWindows>, L"CharonClassName"),
      window(instanceHandle, windowClass, this, WMAPP_Init) {}

void DaemonUserInterfaceWindows::run() {
    MSG message{};
    while (GetMessage(&message, NULL, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

LRESULT DaemonUserInterfaceWindows::windowProcImpl(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WMAPP_Init:
        trayIcon = std::make_unique<TrayIcon>(instanceHandle, windowHandle, WMAPP_TrayIconCallback);
        trayIcon->displayNotification(L"Charon has started", L"You can access Charon settings in the notification area.");
        return 0;
    case WM_DESTROY:
        trayIcon = nullptr;
        return 0;
    case WMAPP_TrayIconCallback:
        return trayIcon->handleEvent(message, wParam, lParam);
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_SHOW_CONFIG_FILE:
            openConfigFile();
            return 0;
        case IDM_EXIT:
            window.destroyWindow();
            stop();
            return 0;
        default:
            return ::DefWindowProcW(windowHandle, message, wParam, lParam);
        }
    default:
        return ::DefWindowProcW(windowHandle, message, wParam, lParam);
    }
}

void DaemonUserInterfaceWindows::openConfigFile() {
    ShellExecuteW(0, 0, L"D:/Desktop/a.txt", 0, 0, SW_SHOW); // TODO
}
