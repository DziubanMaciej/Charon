#include "charon/user_interface/windows/window.h"
#include "charon/user_interface/windows/window_class.h"

Window::Window(HINSTANCE instanceHandle, const WindowClass &windowClass, void *userData, UINT initMessageValue)
    : initMessageValue(initMessageValue) {
    createWindow(instanceHandle, windowClass, userData);
}

void Window::createWindow(HINSTANCE instanceHandle, const WindowClass &windowClass, void *userData) {
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
                                   userData);
    FATAL_ERROR_IF(!windowHandle); // TODO when will this fail?
}

void Window::destroyWindow() {
    if (getUserDataFromHandle<void>(windowHandle) == nullptr) {
        return;
    }

    setUserDataForHandle<void>(windowHandle, nullptr);
    DestroyWindow(windowHandle);
}
