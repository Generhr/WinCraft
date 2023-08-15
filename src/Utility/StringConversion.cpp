#include "Utility/StringConversion.h"

#include <Windows.h>


namespace StringConversion {

    std::wstring Utf8ToWstring(const std::string& utf8Str) {
        int wstrLength = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, nullptr, 0);
        std::wstring wStr;

        wStr.resize(wstrLength);
        MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, &wStr[0], wstrLength);

        return wStr;
    }

    std::string WstringToUtf8(const std::wstring& wStr) {
        int utf8Length = WideCharToMultiByte(CP_UTF8, 0, wStr.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string utf8Str;

        utf8Str.resize(utf8Length);
        WideCharToMultiByte(CP_UTF8, 0, wStr.c_str(), -1, &utf8Str[0], utf8Length, nullptr, nullptr);

        return utf8Str;
    }

}  // namespace StringConversion
