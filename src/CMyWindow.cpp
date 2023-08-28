#include "CMyWindow.h"  //~ Inlcude `Winsock2.h` before `Windows.h` (https://stackoverflow.com/a/1372836)

#include "Exception.h"
#include "Utility/StringConversion.h"
#include "simpleini.h"

#include <Windows.h>
#include <array>   /* std::array */
#include <codecvt> /* std::codecvt_utf8 */
#include <locale>  /* std::wstring_convert */
#include <psapi.h> /* GetModuleFileNameEx */
#include <sstream> /* std::istringstream */
#include <iostream>

#include "resource.h"

IMPLEMENT_DYNAMIC(CMyWindow, CWnd)

BEGIN_MESSAGE_MAP(CMyWindow,
    CWnd)  //~ User-Defined Handlers:
           //: https://learn.microsoft.com/en-us/cpp/mfc/reference/user-defined-handlers?view=msvc-170
ON_MESSAGE(CALLBACKMESSAGE, &CMyWindow::WindowMessageHandler)
ON_WM_CAPTURECHANGED()

ON_REGISTERED_MESSAGE(SHELLMESSAGE, &CMyWindow::ShellMessageHandler)
END_MESSAGE_MAP()

inline static CSimpleIniA
    ini;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables, misc-use-anonymous-namespace)

CMyWindow::CMyWindow() {
    // Constructor implementation
#ifdef WINCRAFT_DEBUG
    std::cout << "CMyWindow\n";
#endif

    popupMenu.LoadMenu(IDR_MAINMENU);

    if (popupMenu == nullptr) {
        throw Exception(GET_EXCEPTION_FILE,
            GET_EXCEPTION_LINE,
            GET_EXCEPTION_COLUMN,
            L"CMenu::LoadMenu()",
            GET_LAST_ERROR_DESCRIPTION);
    }

    const SI_Error rc = ini.LoadFile("Settings.ini");  //: https://github.com/brofield/simpleini

    if (rc != SI_OK) {
        throw Exception(GET_EXCEPTION_FILE,
            GET_EXCEPTION_LINE,
            GET_EXCEPTION_COLUMN,
            L"CSimpleIniA::LoadFile()",
            L"Failed to load ini file (" + std::to_wstring(rc) + L").");
    }
}

CMyWindow::~CMyWindow() {
    // Destructor implementation
#ifdef WINCRAFT_DEBUG
    std::cout << "~CMyWindow()\n";
#endif

    popupMenu.DestroyMenu();
}

LRESULT CMyWindow::WindowMessageHandler([[maybe_unused]] WPARAM wParam, LPARAM lParam) {
    switch (lParam) {
        case WM_LBUTTONUP:
            std::cout << "WM_LBUTTONUP"
                      << "\n";

            break;
        case WM_RBUTTONUP: {
            SetForegroundWindow();

            POINT cursorPos;
            GetCursorPos(&cursorPos);

#ifndef WINCRAFT_TEST
            popupMenu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN, cursorPos.x, cursorPos.y, this);
#endif

            PostMessage(WM_NULL,
                0,
                0);  //~ You must force a task switch to the application that called `TrackPopupMenu`

            break;
        }
    }

    return 0;
}

BOOL CMyWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
    if (HIWORD(wParam) == BN_CLICKED) {
        switch (LOWORD(wParam)) {
            case IDM_MAINMENU_ITEM4:
#ifdef WINCRAFT_TEST
                // NOLINTBEGIN
                fprintf(stderr, "Success");
                exit(EXIT_SUCCESS);
                // NOLINTEND
#else
                PostQuitMessage(0);
#endif
        }

#ifdef WINCRAFT_DEBUG
        std::cout << "\x1B[31mOnCommand\x1B[0m" << std::endl;
#endif
    }

    return CWnd::OnCommand(wParam, lParam);
}

// Using this to restore the focus away from the taskbar when the menu loses focus because a key like Escape or Alt was
// pressed
void CMyWindow::OnCaptureChanged(CWnd* pWnd) {
    if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000 || GetAsyncKeyState(VK_RBUTTON) & 0x8000)) {
        // Simulate pressing Alt + Escape keys
        keybd_event(VK_MENU, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
        keybd_event(VK_ESCAPE, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
        keybd_event(VK_ESCAPE, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
        keybd_event(VK_MENU, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);

#ifdef WINCRAFT_DEBUG
        std::cout << "\x1B[34mRestored focus!\x1B[0m" << std::endl;
#endif
    }

    return CWnd::OnCaptureChanged(pWnd);
}

LRESULT CMyWindow::ShellMessageHandler(WPARAM wParam,  // NOLINT(readability-convert-member-functions-to-static)
    LPARAM lParam) {
    switch (wParam) {
        case HSHELL_WINDOWCREATED:
            std::cout << "HSHELL_WINDOWCREATED"
                      << "\n";

            break;
        case HSHELL_WINDOWDESTROYED: {
            HWND hWnd = reinterpret_cast<HWND>(lParam);  // NOLINT(performance-no-int-to-ptr)
            auto windowIterator = windows.find(hWnd);

            if (windowIterator != windows.end()) {
                // Window was found in the map, remove it
                windows.erase(windowIterator);

#ifdef WINCRAFT_DEBUG
                std::cout << "\x1B[31mRemoved window\x1B[0m (" << hWnd << ")" << std::endl;
#endif
            }
            break;
        }
        case HSHELL_WINDOWACTIVATED:
        case HSHELL_RUDEAPPACTIVATED: {
#ifdef WINCRAFT_DEBUG
            std::cout << "\x1B[2J";  // ANSI escape code to clear the screen
            std::cout << "\x1B[H";   // Move the cursor to the top-left corner
#endif

            if (UnhookWinEvent(hook)) {
#ifdef WINCRAFT_DEBUG
                std::cout << "\x1B[31mUnhookWinEvent\x1B[0m" << std::endl;
#endif
            }

            HWND hWnd = reinterpret_cast<HWND>(lParam);  // NOLINT(performance-no-int-to-ptr)

            if (IsWindow(hWnd)) {
                // Check if the window is in the `windows` already
                if (windows.find(hWnd) == windows.end()) {
                    DWORD windowPID = 0;
                    GetWindowThreadProcessId(hWnd, &windowPID);

                    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, windowPID);

                    // Set `windowProcessPath`
                    std::array<WCHAR, MAX_PATH> windowProcessPath = {};
                    GetModuleFileNameExW(hProcess, nullptr, windowProcessPath.data(), windowProcessPath.size());

                    CloseHandle(hProcess);

                    // Set `windowTitle`
                    std::array<WCHAR, 256> windowTitle = {};
                    GetWindowTextW(hWnd, windowTitle.data(), windowTitle.size());

                    // Set `windowClassName`
                    std::array<WCHAR, 256> windowClassName = {};
                    GetClassNameW(hWnd, windowClassName.data(), windowClassName.size());

                    // Check if the window is a special popup
                    const std::wstring comparison(windowClassName.data());

                    if (comparison == L"#32770" || comparison == L"OperationStatusWindow") {
                        return 0;
                    }

                    // Set `windowProcessName`. Extract the executable name from the full process path
                    std::wstring windowProcessName =
                        static_cast<std::wstring>(windowProcessPath.data())
                            .substr(static_cast<std::wstring>(windowProcessPath.data()).find_last_of(L'\\') + 1);

                    // Check if the process name is "ApplicationFrameHost.exe"
                    if (_wcsicmp(windowProcessName.c_str(), L"ApplicationFrameHost.exe") == 0) {
                        // If so, replace the process name with the window title and append ".exe" to make it unique
                        windowProcessName = windowTitle.data();
                        windowProcessName += L".exe";
                    }

                    // Attempt to read position values from Setting.ini
                    const std::string value = ini.GetValue("Window Positions",
                        StringConversion::WstringToUtf8(windowProcessName).c_str(),
                        "Scheißdreck I. Hosen");

                    if (value != "Scheißdreck I. Hosen") {
                        windows[hWnd].title = windowTitle.data();
                        windows[hWnd].className = windowClassName.data();
                        windows[hWnd].processName = windowProcessName;
                        windows[hWnd].PID = windowPID;

                        // Create a string stream to tokenize the input
                        std::istringstream iss(value);
                        std::string token;

                        // Tokenize the input using ',' as delimiter
                        for (int i = 0; i < 4 && std::getline(iss, token, ','); i++) {
                            const int value = std::stoi(token);

                            (i == 0) ? windows[hWnd].x = value :
                            (i == 1) ? windows[hWnd].y = value :
                            (i == 2) ? windows[hWnd].width = value :
                                       windows[hWnd].height = value;
                        }
                    }
                }

                auto windowIterator = windows.find(hWnd);

                if (windowIterator != windows.end()) {
                    const CMyWindow::Window& currentWindow = windowIterator->second;

                    hook = SetWinEventHook(EVENT_SYSTEM_MOVESIZEEND,
                        EVENT_SYSTEM_MOVESIZEEND,
                        nullptr,
                        &CMyWindow::PositionWindow,
                        currentWindow.PID,
                        0,
                        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

#ifdef WINCRAFT_DEBUG
                    std::wcout << L"HSHELL_RUDEAPPACTIVATED:\n"
                               << L"    * " << currentWindow.title << L"\n"
                               << L"    * " << currentWindow.className << L"\n"
                               << L"    * " << currentWindow.processName << L"\n"
                               << L"    * " << currentWindow.x << L", " << currentWindow.y << L", "
                               << currentWindow.width << L", " << currentWindow.height << std::endl;
                    std::cout << "\x1B[32mSetWinEventHook\x1B[0m" << std::endl;
#endif

                    WINDOWPLACEMENT windowplacement;
                    ::GetWindowPlacement(hWnd, &windowplacement);

                    if (windowplacement.showCmd == SW_MAXIMIZE) {
#ifdef WINCRAFT_DEBUG
                        std::cout << "\x1B[33mThis window is maximized!\x1B[0m" << std::endl;
#endif
                    }
                    else {
                        ::MoveWindow(hWnd,
                            currentWindow.x,
                            currentWindow.y,
                            currentWindow.width,
                            currentWindow.height,
                            TRUE);
#ifdef WINCRAFT_DEBUG
                        std::cout << "\x1B[32mSet initial position!\x1B[0m" << std::endl;
#endif
                    }
                }
                else {
#ifdef WINCRAFT_DEBUG
                    std::cout << "\x1B[33mNot interested in this window!\x1B[0m" << std::endl;
#endif
                }
            }
            else {
#ifdef WINCRAFT_DEBUG
                std::cout << "\x1B[31mThis is not a valid window!\x1B[0m" << std::endl;
#endif
            }

            break;
        }
        case HSHELL_ACTIVATESHELLWINDOW:
        case HSHELL_GETMINRECT:
        case HSHELL_REDRAW:
        case HSHELL_TASKMAN:
        case HSHELL_LANGUAGE:
        case HSHELL_SYSMENU:
        case HSHELL_ENDTASK:
        case HSHELL_ACCESSIBILITYSTATE:
        case HSHELL_APPCOMMAND:
        case HSHELL_WINDOWREPLACED:
        case HSHELL_WINDOWREPLACING:
        case HSHELL_MONITORCHANGED:
            break;

        default:
            std::cout << "UNKOWN: " << wParam << ", " << lParam << "\n";
    }

    return 0;
}

void CALLBACK CMyWindow::PositionWindow(HWINEVENTHOOK /* hWinEventHook */,
    DWORD /* event */,
    HWND hWnd,
    LONG /* idObject */,
    LONG /* idChild */,
    DWORD /* dwEventThread */,
    DWORD /* dwmsEventTime */) {
    CMyWindow::Window& currentWindow = windows.find(hWnd)->second;

    if ((GetKeyState(VK_SHIFT) & 0x8000)) {
        RECT rect;
        ::GetWindowRect(hWnd, &rect);

        currentWindow.x = rect.left;
        currentWindow.y = rect.top;
        currentWindow.width = rect.right - rect.left;
        currentWindow.height = rect.bottom - rect.top;

        if ((GetKeyState(VK_CONTROL) & 0x8000)) {
            // Convert the position values to a string format
            std::ostringstream newValueStream;
            newValueStream << currentWindow.x << ", " << currentWindow.y << ", " << currentWindow.width << ", "
                           << currentWindow.height;
            const std::string newValue = newValueStream.str();

            ini.SetValue("Window Positions",
                StringConversion::WstringToUtf8(currentWindow.processName).c_str(),
                newValue.c_str());
            // Save the changes to the INI file
            ini.SaveFile("Settings.ini");
        }

#ifdef WINCRAFT_DEBUG
        std::cout << "\x1B[34mSaved new window position\x1B[0m" << std::endl;
#endif
    }
    else {
        ::MoveWindow(hWnd, currentWindow.x, currentWindow.y, currentWindow.width, currentWindow.height, TRUE);

#ifdef WINCRAFT_DEBUG
        std::cout << "\x1B[32mRestored window position\x1B[0m" << std::endl;
#endif
    }
}
