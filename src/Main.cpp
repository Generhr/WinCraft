#include "CMyWindow.h"
#include "NotificationIconManager.h"

#include <sstream>
#include <iostream>

void HandleException(const Exception& error, const wchar_t* context) {
    const std::wstringstream ss(L"\n(\x1b[36m" + error.GetFile() + L":" + error.GetLine() + L":" + error.GetColumn() +
                                L"\x1b[0m) \x1b[31mERROR\x1b[0m: " + error.GetNote());
    const std::wstring eMsg = error.GetFullMessage() + L"\n\nException caught at " + context;

    std::wcerr << ss.str() << std::endl;
    MessageBoxW(nullptr, eMsg.c_str(), error.GetTitle().c_str(), MB_OK);
}

void HandleException(const std::exception& error, const wchar_t* context) {
    // Need to convert std::exception what() string from narrow to wide string
    const std::string whatStr(error.what());
    const std::wstring eMsg = std::wstring(whatStr.begin(), whatStr.end()) + L"\n\nException caught at " + context;

    MessageBoxW(nullptr, eMsg.c_str(), L"Unhandled STL Exception", MB_OK);
}

int main() {
    try {
        // Initialize MFC
        if (!AfxWinInit(GetModuleHandle(nullptr), nullptr, GetCommandLine(), SW_HIDE)) {
            throw Exception(GET_EXCEPTION_FILE,
                GET_EXCEPTION_LINE,
                GET_EXCEPTION_COLUMN,
                L"AfxWinInit()",
                L"MFC failed to initialize!");
        }

        CMyWindow myWindow;  // Create an instance of your custom window class

        SetErrorMode(SEM_FAILCRITICALERRORS);

        // Create the window using CreateEx
        if (!myWindow.CreateEx(0,
                AfxRegisterWndClass(0),
                _T("WinCraft"),
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                nullptr,
                nullptr,
                nullptr)) {
            throw Exception(GET_EXCEPTION_FILE,
                GET_EXCEPTION_LINE,
                GET_EXCEPTION_COLUMN,
                L"CMyWindow::CreateEx()",
                GET_LAST_ERROR_DESCRIPTION);
        }

        HWND hWnd = myWindow.GetSafeHwnd();

        NotificationIconManager iconManager(hWnd);

        iconManager.AddNotificationIcon();

        // Register for shell messages using RegisterShellHookWindow()
        RegisterShellHookWindow(hWnd);

        try {
            // Enter the message loop
            MSG message;

            while (GetMessage(&message, nullptr, 0, 0)) {
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
        }
        catch (const Exception& error) {
            HandleException(error, L"message loop");
        }
        catch (const std::exception& error) {
            HandleException(error, L"message loop");
        }
        catch (...) {
            MessageBoxW(nullptr, L"\n\nException caught at message loop.", L"Unhandled Non-STL Exception", MB_OK);
        }

        iconManager.RemoveNotificationIcon();
    }
    catch (const Exception& error) {
        HandleException(error, L"main window creation");
    }
    catch (const std::exception& error) {
        HandleException(error, L"main window creation");
    }
    catch (...) {
        MessageBoxW(nullptr, L"\n\nException caught at main window creation.", L"Unhandled Non-STL Exception", MB_OK);
    }

    return 0;
}
