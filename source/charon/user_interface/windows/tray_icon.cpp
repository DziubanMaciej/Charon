#include "charon/user_interface/windows/tray_icon.h"
#include "charon/util/error.h"
#include "charon_resources/windows/resource.h"

#include <commctrl.h>
#include <shellapi.h>

#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

TrayIcon::TrayIcon(HINSTANCE instanceHandle, HWND windowHandle, UINT callbackMessageValue)
    : instanceHandle(instanceHandle),
      windowHandle(windowHandle),
      callbackMessageValue(callbackMessageValue) {
    addNotificationIcon();
}

TrayIcon::~TrayIcon() {
    deleteNotificationIcon();
}

LRESULT TrayIcon::handleEvent(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (LOWORD(lParam)) {
    case NIN_SELECT:
        return 0;

    case NIN_BALLOONTIMEOUT:
        return 0;

    case NIN_BALLOONUSERCLICK:
        return 0;

    case WM_CONTEXTMENU:
        showContextMenu(wParam, lParam);
        return 0;
    default:
        return ::DefWindowProcW(windowHandle, message, wParam, lParam);
    }
}

void TrayIcon::addNotificationIcon() {
    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = windowHandle;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_GUID;
    nid.guidItem = __uuidof(TrayIconGuid);
    nid.uCallbackMessage = callbackMessageValue;
    LoadIconMetric(instanceHandle, MAKEINTRESOURCEW(IDI_ICON), LIM_SMALL, &nid.hIcon);

    BOOL success = Shell_NotifyIconW(NIM_ADD, &nid);
    FATAL_ERROR_IF(!success, "Failed to create tray icon");

    nid.uVersion = NOTIFYICON_VERSION_4;
    success = Shell_NotifyIconW(NIM_SETVERSION, &nid);
    FATAL_ERROR_IF(!success, "Failed to set tray icon version");
}

void TrayIcon::deleteNotificationIcon() {
    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = windowHandle;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_GUID;
    nid.guidItem = __uuidof(TrayIconGuid);
    nid.uCallbackMessage = callbackMessageValue;
    BOOL success = Shell_NotifyIconW(NIM_DELETE, &nid);
    FATAL_ERROR_IF(!success, "Failed to remove tray icon");
}

void TrayIcon::showContextMenu(WPARAM wParam, LPARAM lParam) {
    HMENU hMenu = LoadMenu(instanceHandle, MAKEINTRESOURCE(IDC_TRAY_CONTEXT_MENU));
    if (!hMenu) {
        return;
    }

    HMENU hSubMenu = GetSubMenu(hMenu, 0);
    if (!hSubMenu) {
        return;
    }

    // our window must be foreground before calling TrackPopupMenu or the menu will not disappear when the user clicks away
    SetForegroundWindow(windowHandle);

    // respect menu drop alignment
    UINT uFlags = TPM_RIGHTBUTTON;
    if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0) {
        uFlags |= TPM_RIGHTALIGN;
    } else {
        uFlags |= TPM_LEFTALIGN;
    }

    const POINT pt = {LOWORD(wParam), HIWORD(wParam)};
    TrackPopupMenuEx(hSubMenu, uFlags, pt.x, pt.y, windowHandle, NULL);

    DestroyMenu(hMenu);
}
