#include <iostream>

#include "CMyWindow.h"
#include "NotificationIconManager.h"


int main() {
    CMyWindow myWindow;  // Create an instance of your custom window class

    // Initialize MFC
    if (!AfxWinInit(::GetModuleHandle(nullptr), nullptr, ::GetCommandLine(), SW_HIDE)) {
        throw("MFC failed to initialize!");
    }

    SetErrorMode(SEM_FAILCRITICALERRORS);

    // Create the window using CreateEx
    if (!myWindow.CreateEx(0,
            AfxRegisterWndClass(0),
            _T("Shell Message Example"),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            nullptr,
            nullptr,
            nullptr)) {
        return -1;  // Handle the error here
    }

    HWND hWnd = myWindow.GetSafeHwnd();

    if (!IsWindow(hWnd)) {
        throw("!IsWindow()");
    }

    NotificationIconManager iconManager(hWnd);

    iconManager.AddNotificationIcon();

    // Register for shell messages using RegisterShellHookWindow()
    RegisterShellHookWindow(hWnd);

    // Enter the message loop
    MSG msg;

    //! myWindow.SendMessage(WM_COMMAND, MAKEWPARAM(4007, BN_CLICKED), reinterpret_cast<LPARAM>(nullptr));

    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    iconManager.RemoveNotificationIcon();

    return 0;
}
