#pragma once

#include "charon/util/class_traits.h"

#include <Windows.h>

class __declspec(uuid("d25a9ba1-79a8-405c-9ce0-7678390d1dc7")) TrayIconGuid;

class TrayIcon : NonCopyableAndMovable {
public:
    TrayIcon(HINSTANCE instanceHandle, HWND windowHandle, UINT callbackMessageValue);
    ~TrayIcon();

    LRESULT handleEvent(UINT message, WPARAM wParam, LPARAM lParam);

private:
    void addNotificationIcon();
    void deleteNotificationIcon();

    void showContextMenu(WPARAM wParam, LPARAM lParam);

    const HINSTANCE instanceHandle;
    const HWND windowHandle;
    const UINT callbackMessageValue;
};
