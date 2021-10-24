#pragma once

#include "charon/util/class_traits.h"

#include <Windows.h>
#include <string>

class TrayIcon : NonCopyableAndMovable {
public:
    TrayIcon(HINSTANCE instanceHandle, HWND windowHandle, UINT callbackMessageValue);
    ~TrayIcon();

    LRESULT handleEvent(UINT message, WPARAM wParam, LPARAM lParam);
    void displayNotification(const std::wstring &title, const std::wstring &message);

private:
    void addNotificationIcon();
    void deleteNotificationIcon();

    void showContextMenu(LPARAM lParam);

    const HINSTANCE instanceHandle;
    const HWND windowHandle;
    const UINT callbackMessageValue;
};
