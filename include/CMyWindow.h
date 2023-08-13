#pragma once

#include <winsock2.h> /* Needed by `afxwin.h` */
#include <afxwin.h>
#include <array> /* std::array */


#define CALLBACKMESSAGE (WM_APP + 1)
#define __DEBUG

class CMyWindow
    : public CWnd {  //~ CWnd Class:
                     //: https://github.com/MicrosoftDocs/cpp-docs/blob/main/docs/mfc/reference/cwnd-class.md#onmenuselect
public:
    DECLARE_DYNAMIC(CMyWindow)  // cppcheck-suppress unknownMacro

    CMyWindow();
    ~CMyWindow() override;

    CMyWindow(const CMyWindow&) = delete;             // Copy constructor
    CMyWindow& operator=(const CMyWindow&) = delete;  // Copy assignment operator
    CMyWindow(CMyWindow&&) = delete;                  // Move constructor
    CMyWindow& operator=(CMyWindow&&) = delete;       // Move assignment operator

protected:
    DECLARE_MESSAGE_MAP()

private:
    LRESULT WindowMessageHandler(WPARAM wParam, LPARAM lParam);
    LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

    LRESULT ShellMessageHandler(WPARAM wParam, LPARAM lParam);
    static void CALLBACK PositionWindow(HWINEVENTHOOK hWinEventHook,
        DWORD event,
        HWND hwnd,
        LONG idObject,
        LONG idChild,
        DWORD dwEventThread,
        DWORD dwmsEventTime);

    inline const static int SHELLMESSAGE = RegisterWindowMessage("SHELLHOOK");

    inline static HWINEVENTHOOK hook;
    inline static CWnd* pWnd;
    inline static std::array<int, 4> position;
};
