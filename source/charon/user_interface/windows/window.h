#pragma once

#include "charon/util/class_traits.h"

#include <Windows.h>

class WindowClass;

class Window : NonCopyableAndMovable {
public:
    Window(HINSTANCE instanceHandle, const WindowClass &windowClass, void *userData, UINT initMessageValue);

    // Called directly by win32
    template <typename UserData>
    static LRESULT CALLBACK windowProcNative(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

    void destroyWindow();

private:
    void createWindow(HINSTANCE instanceHandle, const WindowClass &windowClass, void *userData);

    template <typename UserData>
    static void setUserDataForHandle(HWND windowHandle, UserData *userData);
    template <typename UserData>
    static UserData *getUserDataFromHandle(HWND windowHandle);
    template <typename UserData>
    static UserData *getUserData(HWND windowHandle, UINT message, LPARAM lParam);

    const UINT initMessageValue{};
    HWND windowHandle{};
};

template <typename UserData>
LRESULT Window::windowProcNative(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) {
    // Get window instance
    UserData *userData = getUserData<UserData>(windowHandle, message, lParam);

    // Handle destruction
    if (message == WM_DESTROY) {
        setUserDataForHandle<UserData>(windowHandle, nullptr);
        ::PostQuitMessage(0);
    }

    // If there is no window, it's been destroyed, ignore
    if (userData == nullptr) {
        return ::DefWindowProcW(windowHandle, message, wParam, lParam);
    }

    // Process actual, normal events
    return userData->windowProcImpl(windowHandle, message, wParam, lParam);
}

template <typename UserData>
inline void Window::setUserDataForHandle(HWND windowHandle, UserData *userData) {
    SetWindowLongPtr(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(userData));
}

template <typename UserData>
inline UserData *Window::getUserDataFromHandle(HWND windowHandle) {
    return reinterpret_cast<UserData *>(GetWindowLongPtr(windowHandle, GWLP_USERDATA));
}

template <typename UserData>
inline UserData *Window::getUserData(HWND windowHandle, UINT message, LPARAM lParam) {
    UserData *result = {};
    if (message == WM_CREATE) {
        result = reinterpret_cast<UserData *>(reinterpret_cast<CREATESTRUCT *>(lParam)->lpCreateParams);
        setUserDataForHandle<UserData>(windowHandle, result);
        PostMessage(windowHandle, UserData::WMAPP_Init, 0, 0);
    } else {
        result = getUserDataFromHandle<UserData>(windowHandle);
    }
    return result;
}
