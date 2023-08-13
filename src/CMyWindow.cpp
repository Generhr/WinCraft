#include "CMyWindow.h"  //~ Inlcude `winsock2.h` before `windows.h` (https://stackoverflow.com/a/1372836)

#include <windows.h>
#include <psapi.h> /* GetModuleFileNameEx */
#include <string>  /* std::wstring */
#include <sstream> /* std::istringstream */
#include <locale>  /* std::wstring_convert */
#include <codecvt> /* std::codecvt_utf8_utf16 */
#include <iostream>

#include "simpleini.h"


IMPLEMENT_DYNAMIC(CMyWindow, CWnd)

int SHELLMESSAGE = RegisterWindowMessage("SHELLHOOK");

BEGIN_MESSAGE_MAP(CMyWindow,
    CWnd)  //~ User-Defined Handlers:
           //: https://learn.microsoft.com/en-us/cpp/mfc/reference/user-defined-handlers?view=msvc-170
ON_MESSAGE(CALLBACKMESSAGE, &CMyWindow::WindowMessageHandler)
ON_REGISTERED_MESSAGE(SHELLMESSAGE, &CMyWindow::ShellMessageHandler)
END_MESSAGE_MAP()

inline static CSimpleIniA ini;

CMyWindow::CMyWindow() {
    // Constructor implementation
    std::cout << "CMyWindow"
              << "\n";

    SI_Error rc = ini.LoadFile("Settings.ini");  //: https://github.com/brofield/simpleini
    assert(rc == SI_OK);
}

CMyWindow::~CMyWindow() {
    // Destructor implementation
    std::cout << "~CMyWindow()"
              << "\n";
}

#define IDM_MENU_ITEM1 1001
#define IDM_MENU_ITEM2 1002
#define IDM_MENU_ITEM3 1003
#define IDM_MENU_ITEM4 1004

afx_msg LRESULT CMyWindow::WindowMessageHandler([[maybe_unused]] WPARAM wParam, LPARAM lParam) {
    switch (lParam) {
        case WM_LBUTTONUP:
            std::cout << "WM_LBUTTONUP"
                      << "\n";

            break;
        case WM_RBUTTONUP: {
            // Create and display the context menu
            CMenu SubMenu, MainMenu;

            SubMenu.CreatePopupMenu();
            SubMenu.AppendMenu(MF_STRING, 4001, _T("1"));
            SubMenu.AppendMenu(MF_STRING, 4002, _T("2"));
            SubMenu.AppendMenu(MF_STRING, 4003, _T("4"));
            SubMenu.AppendMenu(MF_STRING, 4004, _T("8"));

            MainMenu.CreatePopupMenu();
            MainMenu.AppendMenu(MF_STRING, 4005, _T("Line"));
            MainMenu.AppendMenu(MF_STRING, 4006, _T("Circle"));
            MainMenu.AppendMenuA(MF_POPUP, reinterpret_cast<UINT_PTR>(SubMenu.GetSafeHmenu()), _T("Line Thickness"));
            MainMenu.AppendMenu(MF_STRING, 4007, _T("Exit"));

            POINT cursorPos;
            GetCursorPos(&cursorPos);

            MainMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN, cursorPos.x, cursorPos.y, this);

            break;
        }
    }

    return 0;
}

LRESULT CMyWindow::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_COMMAND) {
        if (HIWORD(wParam) == BN_CLICKED) {
            switch (LOWORD(wParam)) {
                case 4007:
                    PostQuitMessage(0);
            }
        }
    }

    return CWnd::WindowProc(message, wParam, lParam);
}

LRESULT CMyWindow::ShellMessageHandler(WPARAM wParam, LPARAM lParam) {
    switch (wParam) {
        case HSHELL_WINDOWCREATED:
            std::cout << "HSHELL_WINDOWCREATED"
                      << "\n";

            break;
        case HSHELL_WINDOWDESTROYED:
            break;
        case HSHELL_WINDOWACTIVATED:
        case HSHELL_RUDEAPPACTIVATED: {
#ifdef __DEBUG
            std::cout << "\x1B[2J";  // ANSI escape code to clear the screen
            std::cout << "\x1B[H";   // Move the cursor to the top-left corner
#endif

            if (UnhookWinEvent(hook)) {
#ifdef __DEBUG
                std::wcout << L"\x1B[31mUnhookWinEvent\x1B[0m" << std::endl;
#endif
            }

            HWND hWnd = reinterpret_cast<HWND>(lParam);

            if (pWnd = CMyWindow::FromHandle(hWnd)) {
                DWORD dwPID;
                GetWindowThreadProcessId(hWnd, &dwPID);

                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPID);

                // Set `windowProcessPath`
                std::array<WCHAR, MAX_PATH> windowProcessPath = {};
                GetModuleFileNameExW(hProcess, nullptr, windowProcessPath.data(), windowProcessPath.size());

                CloseHandle(hProcess);

                // Set `windowTitle`
                std::array<WCHAR, 256> windowTitle = {};
                GetWindowTextW(hWnd, windowTitle.data(), windowTitle.size());

                // Set `windowClass`
                std::array<WCHAR, 256> windowClass = {};
                GetClassNameW(hWnd, windowClass.data(), windowClass.size());

                // Set `windowProcessName`
                std::wstring windowProcessName =
                    static_cast<std::wstring>(windowProcessPath.data())
                        .substr(static_cast<std::wstring>(windowProcessPath.data()).find_last_of(L'\\') + 1);

                if (_wcsicmp(windowProcessName.c_str(), L"ApplicationFrameHost.exe") == 0) {
                    windowProcessName = windowTitle.data();
                    windowProcessName += L".exe";
                }

                // Attempt to read position values from Setting.ini
                std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                std::string narrowWindowProcessName = converter.to_bytes(windowProcessName);

                std::string value = ini.GetValue("Window Positions", narrowWindowProcessName.c_str(), "default");

                if (value != "default") {
                    // Create a string stream to tokenize the input
                    std::istringstream iss(value);
                    std::string token;

                    // Tokenize the input using ',' as delimiter
                    for (int i = 0; i < 4 && std::getline(iss, token, ','); i++) {
                        // Convert the token to an integer and store it in the array
                        position[i] = std::stoi(token);
                    }

                    DWORD dwProcessId;
                    GetWindowThreadProcessId(hWnd, &dwProcessId);

                    hook = SetWinEventHook(EVENT_SYSTEM_MOVESIZEEND,
                        EVENT_SYSTEM_MOVESIZEEND,
                        NULL,
                        &CMyWindow::PositionWindow,
                        dwProcessId,
                        0,
                        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

#ifdef __DEBUG
                    std::wcout << L"HSHELL_RUDEAPPACTIVATED:\n"
                               << L"    * " << windowTitle.data() << L"\n"
                               << L"    * " << windowClass.data() << L"\n"
                               << L"    * " << windowProcessName << L"\n"
                               << L"    * " << position[0] << L", " << position[1] << L", " << position[2] << L", "
                               << position[3] << std::endl;
                    std::wcout << L"\x1B[32mSetWinEventHook\x1B[0m" << std::endl;
#endif

                    WINDOWPLACEMENT wp;
                    pWnd->GetWindowPlacement(&wp);

                    if (wp.showCmd == SW_MAXIMIZE) {
#ifdef __DEBUG
                        std::cout << "\x1B[33mWindow is maximized\x1B[0m" << std::endl;
#endif
                    }
                    else {
                        pWnd->MoveWindow(position[0], position[1], position[2], position[3]);
#ifdef __DEBUG
                        std::cout << "\x1B[32mSet initial position\x1B[0m" << std::endl;
#endif
                    }
                }
                else {
#ifdef __DEBUG
                    std::wcout << L"\x1B[33mNot interested in this window!\x1B[0m" << std::endl;
#endif
                }
            }
            else {
#ifdef __DEBUG
                std::wcout << L"\x1B[31mThis is not a valid window!\x1B[0m" << std::endl;
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
    if (GetKeyState(VK_SHIFT) & 0x8000) {
        DWORD dwPID;
        GetWindowThreadProcessId(hWnd, &dwPID);

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPID);

        // Set `windowProcessPath`
        std::array<WCHAR, MAX_PATH> windowProcessPath = {};
        GetModuleFileNameExW(hProcess, nullptr, windowProcessPath.data(), windowProcessPath.size());

        CloseHandle(hProcess);

        // Set `windowTitle`
        std::array<WCHAR, 256> windowTitle = {};
        GetWindowTextW(hWnd, windowTitle.data(), windowTitle.size());

        // Set `windowProcessName`
        std::wstring windowProcessName =
            static_cast<std::wstring>(windowProcessPath.data())
                .substr(static_cast<std::wstring>(windowProcessPath.data()).find_last_of(L'\\') + 1);

        if (_wcsicmp(windowProcessName.c_str(), L"ApplicationFrameHost.exe") == 0) {
            windowProcessName = windowTitle.data();
            windowProcessName += L".exe";
        }

        RECT rect;
        pWnd->GetWindowRect(&rect);

        position[0] = rect.left;
        position[1] = rect.top;
        position[2] = rect.right - rect.left;
        position[3] = rect.bottom - rect.top;

        // Convert the position values to a string format
        std::ostringstream newValueStream;
        newValueStream << position[0] << ", " << position[1] << ", " << position[2] << ", " << position[3];
        std::string newValue = newValueStream.str();

        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        // Convert the process name to narrow string
        std::string narrowWindowProcessName = converter.to_bytes(windowProcessName);

        ini.SetValue("Window Positions", narrowWindowProcessName.c_str(), newValue.c_str());

        if (GetKeyState(VK_CONTROL) & 0x8000) {
            // Save the changes to the INI file
            ini.SaveFile("Settings.ini");
        }

#ifdef __DEBUG
        std::cout << "\x1B[34mSaved new window position\x1B[0m" << std::endl;
#endif
    }
    else {
        pWnd->MoveWindow(position[0], position[1], position[2], position[3]);

#ifdef __DEBUG
        std::cout << "\x1B[32mRestored window position\x1B[0m" << std::endl;
#endif
    }
}
