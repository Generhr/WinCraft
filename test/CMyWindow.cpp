#include "CMyWindow.h"

#include <gtest/gtest.h>

#include "resource.h"

#define CREATE_WINDOW                                                                                                  \
    AfxWinInit(GetModuleHandle(nullptr), nullptr, GetCommandLine(), SW_HIDE);                                          \
    CMyWindow myWindow;                                                                                                \
    myWindow.CreateEx(0,                                                                                               \
        AfxRegisterWndClass(0),                                                                                        \
        _T("WinCraft"),                                                                                                \
        WS_OVERLAPPEDWINDOW,                                                                                           \
        CW_USEDEFAULT,                                                                                                 \
        CW_USEDEFAULT,                                                                                                 \
        CW_USEDEFAULT,                                                                                                 \
        CW_USEDEFAULT,                                                                                                 \
        nullptr,                                                                                                       \
        nullptr,                                                                                                       \
        nullptr);

TEST(CMyWindow, CreateDestroyWindow) {
    CREATE_WINDOW;

    HWND hWnd = myWindow.GetSafeHwnd();

    EXPECT_TRUE(IsWindow(hWnd));

    myWindow
        .DestroyWindow();  //~ Destroying window objects:
                           //: https://learn.microsoft.com/en-us/cpp/mfc/tn017-destroying-window-objects?view=msvc-170

    EXPECT_FALSE(IsWindow(hWnd));
}

TEST(CMyWindow, MessageHandling) {
    CREATE_WINDOW;

    ::testing::FLAGS_gtest_death_test_style = "threadsafe";

    EXPECT_EXIT(
        {
            myWindow.SendMessage(CALLBACKMESSAGE, reinterpret_cast<WPARAM>(nullptr), static_cast<LPARAM>(WM_RBUTTONUP));
            myWindow.SendMessage(WM_COMMAND,
                MAKEWPARAM(IDM_MAINMENU_ITEM4, BN_CLICKED),
                reinterpret_cast<LPARAM>(nullptr));
            fprintf(stderr, "Failure");
            exit(EXIT_FAILURE);
        },
        testing::ExitedWithCode(EXIT_SUCCESS),
        "Success");
}
