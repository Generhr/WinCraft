#include "Utility/StringConversion.h"  // Include the header that defines Utf8ToWstring and WstringToUtf8 functions

#include <gtest/gtest.h>


TEST(StringConversionTest, Utf8ToWstring) {
    // Test converting an empty string
    std::string emptyUtf8Str = "";
    std::wstring emptyWStr = StringConversion::Utf8ToWstring(emptyUtf8Str);
#ifdef _WIN32
    // On Windows, the null terminator is included, so remove it for the comparison
    emptyWStr.pop_back();
#endif

    EXPECT_EQ(emptyWStr, L"");

    // Test converting a simple UTF-8 string
    std::string utf8Str = "Hello, 你好";
    std::wstring wStr = StringConversion::Utf8ToWstring(utf8Str);
#ifdef _WIN32
    wStr.pop_back();
#endif

    EXPECT_EQ(wStr, L"Hello, 你好");

    // Test converting a UTF-8 string with special characters
    std::string specialUtf8Str = "© $ €";
    std::wstring specialWStr = StringConversion::Utf8ToWstring(specialUtf8Str);
#ifdef _WIN32
    specialWStr.pop_back();
#endif

    EXPECT_EQ(specialWStr, L"© $ €");
}

TEST(StringConversionTest, WstringToUtf8) {
    // Test converting an empty wide string
    std::wstring emptyWStr = L"";
    std::string emptyUtf8Str = StringConversion::WstringToUtf8(emptyWStr);
#ifdef _WIN32
    emptyUtf8Str.pop_back();
#endif

    EXPECT_EQ(emptyUtf8Str, "");

    // Test converting a simple wide string
    std::wstring wStr = L"Hello, 你好";
    std::string utf8Str = StringConversion::WstringToUtf8(wStr);
#ifdef _WIN32
    utf8Str.pop_back();
#endif

    EXPECT_EQ(utf8Str, "Hello, 你好");

    // Test converting a wide string with special characters
    std::wstring specialWStr = L"© $ €";
    std::string specialUtf8Str = StringConversion::WstringToUtf8(specialWStr);
#ifdef _WIN32
    specialUtf8Str.pop_back();
#endif

    EXPECT_EQ(specialUtf8Str, "© $ €");
}
