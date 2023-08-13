#pragma once

#include <windows.h>
#include <shellapi.h> /* NOTIFYICONDATAA, Shell_NotifyIcon */


class NotificationIconManager {
public:
    explicit NotificationIconManager(HWND hWnd);
    ~NotificationIconManager();

    NotificationIconManager(const NotificationIconManager&) = delete;
    NotificationIconManager& operator=(const NotificationIconManager&) = delete;
    NotificationIconManager(NotificationIconManager&&) = delete;
    NotificationIconManager& operator=(NotificationIconManager&&) = delete;

    void AddNotificationIcon();
    void RemoveNotificationIcon();

private:
    HWND hWnd;
    NOTIFYICONDATAA nid;
};
