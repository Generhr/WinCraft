#pragma once

#include <Winsock2.h> /* Needed by `afxwin.h` */
#include <afxwin.h>
#include <string> /* std::wstring */
#include <unordered_map>


#define CALLBACKMESSAGE (WM_APP + 1)

class CMyWindow : public CWnd {  //~ CWnd Class:
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
    inline const static int SHELLMESSAGE = RegisterWindowMessage("SHELLHOOK");

    inline static CMenu popupMenu;

    LRESULT WindowMessageHandler(WPARAM wParam, LPARAM lParam);
    BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    void OnCaptureChanged(CWnd* pWnd);

    struct Window {
        std::wstring title;
        std::wstring className;
        std::wstring processName;
        DWORD PID;

        int x;
        int y;
        int width;
        int height;
    };

    inline static HWINEVENTHOOK hook;
    inline static std::unordered_map<HWND, Window> windows;

    LRESULT ShellMessageHandler(WPARAM wParam, LPARAM lParam);
    static void CALLBACK
    PositionWindow(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
};
