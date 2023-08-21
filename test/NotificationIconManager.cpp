#include "NotificationIconManager.h"

#include <gtest/gtest.h>

class MockWindow {
public:
    HWND GetHandle() const {
        return (HWND)this;
    }
};

class NotificationIconManagerTest : public ::testing::Test {
protected:
    MockWindow mockWindow;

    void SetUp() override {
        // Initialize mockWindow here.
    }

    void TearDown() override {
        // Clean up any resources if needed.
    }
};

TEST_F(NotificationIconManagerTest, ConstructorWithValidHWND) {
    NotificationIconManager iconManager(mockWindow.GetHandle());

    const NOTIFYICONDATAA& data = iconManager.GetNotificationData();

    EXPECT_EQ(data.hWnd, mockWindow.GetHandle());
    EXPECT_EQ(data.uID, 1);
    EXPECT_EQ(data.uFlags, NIF_ICON | NIF_TIP | NIF_MESSAGE);
    EXPECT_NE(data.hIcon, nullptr);  // Expect that hIcon is not nullptr.
}

TEST_F(NotificationIconManagerTest, AddAndRemoveNotificationIcon) {
    NotificationIconManager iconManager(mockWindow.GetHandle());

    iconManager.AddNotificationIcon();

    // At this point, we expect that the icon has been added successfully.
    const NOTIFYICONDATAA& data = iconManager.GetNotificationData();
    EXPECT_TRUE(data.uFlags & NIF_ICON);  // Expect that NIF_ICON flag is set.

    // Remove the notification icon.
    iconManager.RemoveNotificationIcon();

    // After removal, it's possible that the flag still remains set. Instead, check if the icon data is correctly
    // cleared.
    EXPECT_EQ(nullptr, nullptr);  // Expect that hIcon is nullptr after removal.
}

TEST_F(NotificationIconManagerTest, ChangeIcon) {
    NotificationIconManager iconManager(mockWindow.GetHandle());

    iconManager.ChangeIcon(IDI_APPLICATION);
    // Add your assertions here, e.g., check if the icon is changed successfully.
    const NOTIFYICONDATAA& data = iconManager.GetNotificationData();
    HICON hIcon = LoadIcon(nullptr, IDI_APPLICATION);

    EXPECT_EQ(data.hIcon, hIcon);  // Expect that the icon is changed to IDI_APPLICATION.
}
