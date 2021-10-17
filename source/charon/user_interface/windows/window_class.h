#pragma once

#include "charon/util/error.h"
#include "charon_resources/windows/resource.h"

#include <Windows.h>

class WindowClass {
public:
    WindowClass(HINSTANCE instanceHandle, WNDPROC windowProc, const std::wstring &className)
        : instanceHandle(instanceHandle),
          className(className) {
        WNDCLASSEXW data = {sizeof(data)};
        data.style = CS_HREDRAW | CS_VREDRAW;
        data.lpfnWndProc = windowProc;
        data.hInstance = GetModuleHandle(NULL);
        data.hIcon = LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_ICON));
        data.hCursor = LoadCursor(NULL, IDC_ARROW);
        data.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        data.lpszMenuName = NULL;
        data.lpszClassName = getClassName();
        ATOM retVal = RegisterClassExW(&data);
        FATAL_ERROR_IF(retVal == 0); // TODO when will this fail?
    }

    ~WindowClass() {
        UnregisterClassW(getClassName(), instanceHandle);
    }

    LPCWSTR getClassName() const {
        return className.c_str();
    }

private:
    const HINSTANCE instanceHandle;
    const std::wstring className;
};
