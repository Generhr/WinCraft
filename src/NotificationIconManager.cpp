#include <strsafe.h> /* StringCchCopyA */
#include <gsl/gsl>   /* gsl::span, gsl::ensure_z */

#include "CMyWindow.h" /* CALLBACKMESSAGE */
#include "NotificationIconManager.h"

NotificationIconManager::NotificationIconManager(HWND hWnd) : hWnd(hWnd), nid() {
    nid.cbSize = sizeof(NOTIFYICONDATAA);
    nid.hWnd = hWnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    nid.uCallbackMessage = CALLBACKMESSAGE;
    nid.hIcon = LoadIcon(nullptr, IDI_SHIELD);

    const gsl::span<const char> tipArrayView = gsl::ensure_z("WinCraft");
    StringCchCopyA(std::begin(nid.szTip), sizeof(nid.szTip), static_cast<STRSAFE_LPCSTR>(tipArrayView.data()));
}

NotificationIconManager::~NotificationIconManager() {
    RemoveNotificationIcon();
}

void NotificationIconManager::AddNotificationIcon() {
    Shell_NotifyIcon(NIM_ADD, &nid);
}

void NotificationIconManager::RemoveNotificationIcon() {
    Shell_NotifyIcon(NIM_DELETE, &nid);
}
