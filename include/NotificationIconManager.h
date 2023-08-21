#pragma once

#include "Exception.h"

#include <windows.h>
#include <shellapi.h> /* NOTIFYICONDATAA, Shell_NotifyIcon */

class NotificationIconManager {
public:
    NotificationIconManager() = default;
    explicit NotificationIconManager(HWND hWnd);
    ~NotificationIconManager();

    NotificationIconManager(const NotificationIconManager&) = delete;
    NotificationIconManager& operator=(const NotificationIconManager&) = delete;
    NotificationIconManager(NotificationIconManager&&) = delete;
    NotificationIconManager& operator=(NotificationIconManager&&) = delete;

    void AddNotificationIcon();
    void RemoveNotificationIcon();

    void ChangeIcon(LPCSTR iconName) {
        HICON hIcon = LoadIconA(nullptr, iconName);

        if (hIcon != nullptr) {
            nid.hIcon = hIcon;

            Shell_NotifyIcon(NIM_MODIFY, &nid);
        }
        else {
        }
    }

    [[nodiscard]] const NOTIFYICONDATAA& GetNotificationData() const {
        return nid;
    }

private:
    HWND hWnd;
    NOTIFYICONDATAA nid;
};
