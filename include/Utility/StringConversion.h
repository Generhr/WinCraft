#pragma once

#include <codecvt>
#include <locale>
#include <string>

namespace StringConversion {

    // Convert a UTF-8 encoded string to a wide string
    std::wstring Utf8ToWstring(const std::string& utf8Str);

    // Convert a wide string to a UTF-8 encoded string
    std::string WstringToUtf8(const std::wstring& wStr);

}  // namespace StringConversion
