
#include "charon/user_interface/windows/daemon_user_interface_windows.h"
#include "charon/util/error.h"

#include <shellapi.h>

std::unique_ptr<DaemonUserInterface> DaemonUserInterface::create(Charon &charon) {
    return std::unique_ptr<DaemonUserInterface>(new DaemonUserInterfaceWindows(charon));
}

DaemonUserInterfaceWindows::DaemonUserInterfaceWindows(Charon &charon)
    : DaemonUserInterface(charon),
      instanceHandle(GetModuleHandle(NULL)),
      windowClass(instanceHandle, windowProcNative, L"CharonClassName") {}

void DaemonUserInterfaceWindows::run() {
    createWindow();

    MSG message{};
    while (GetMessage(&message, NULL, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    destroyWindow();
}

void DaemonUserInterfaceWindows::createWindow() {
    windowHandle = CreateWindowExW(0u,
                                   windowClass.getClassName(),
                                   L"",
                                   WS_OVERLAPPEDWINDOW,
                                   CW_USEDEFAULT,
                                   0,
                                   250,
                                   200,
                                   NULL,
                                   NULL,
                                   instanceHandle,
                                   this);
    FATAL_ERROR_IF(!windowHandle); // TODO when will this fail?
}

void DaemonUserInterfaceWindows::destroyWindow() {
    if (getUserDataFromHandle(windowHandle) == nullptr) {
        return;
    }

    setUserDataForHandle(windowHandle, nullptr);
    DestroyWindow(windowHandle);
}

void DaemonUserInterfaceWindows::setUserDataForHandle(HWND windowHandle, DaemonUserInterfaceWindows *windowProcWithDataForHandle) {
    SetWindowLongPtr(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(windowProcWithDataForHandle));
}

DaemonUserInterfaceWindows *DaemonUserInterfaceWindows::getUserDataFromHandle(HWND windowHandle) {
    return reinterpret_cast<DaemonUserInterfaceWindows *>(GetWindowLongPtr(windowHandle, GWLP_USERDATA));
}

DaemonUserInterfaceWindows *DaemonUserInterfaceWindows::getUserData(HWND windowHandle, UINT message, LPARAM lParam) {
    DaemonUserInterfaceWindows *result = {};
    if (message == WM_CREATE) {
        result = reinterpret_cast<DaemonUserInterfaceWindows *>(reinterpret_cast<CREATESTRUCT *>(lParam)->lpCreateParams);
        setUserDataForHandle(windowHandle, result);
    } else {
        result = getUserDataFromHandle(windowHandle);
    }
    return result;
}

LRESULT DaemonUserInterfaceWindows::windowProcNative(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) {
    // Get window instance
    DaemonUserInterfaceWindows *userData = getUserData(windowHandle, message, lParam);

    // Process destruction
    if (message == WM_DESTROY) {
        setUserDataForHandle(windowHandle, nullptr);
        ::PostQuitMessage(0);
    }

    // If there is no window, it's been destroyed, ignore
    if (userData == nullptr) {
        return ::DefWindowProcW(windowHandle, message, wParam, lParam);
    }

    // Process actual, normal events
    return userData->windowProcImpl(windowHandle, message, wParam, lParam);
}

LRESULT DaemonUserInterfaceWindows::windowProcImpl(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        trayIcon = std::make_unique<TrayIcon>(instanceHandle, windowHandle, WMAPP_TrayIconCallback);
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
            destroyWindow();
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
    ShellExecuteW(0, 0, L"D:/Desktop/a.txt", 0, 0, SW_SHOW);
}
