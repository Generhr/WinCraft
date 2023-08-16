#pragma once

#include "Utility/StringConversion.h"

#include <string>
#include <iostream>


#define GET_EXCEPTION_FILE __FILEW__                              //! StringConversion::Utf8ToWstring(std::source_location::current().file_name())
#define GET_EXCEPTION_LINE std::to_wstring(__LINE__)              //! std::to_wstring(std::source_location::current().line())
#define GET_EXCEPTION_COLUMN std::to_wstring(__builtin_COLUMN())  //! std::to_wstring(std::source_location::current().column())

#define GET_LAST_ERROR_DESCRIPTION                                                                                                                   \
    []() -> std::wstring {                                                                                                                           \
        DWORD errorCode = GetLastError();                                                                                                            \
        LPWSTR errorMsgBuffer = nullptr;                                                                                                             \
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,                                                                  \
            nullptr,                                                                                                                                 \
            errorCode,                                                                                                                               \
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),                                                                                               \
            reinterpret_cast<LPWSTR>(&errorMsgBuffer),                                                                                               \
            0,                                                                                                                                       \
            nullptr);                                                                                                                                \
        std::wstring errorDescription = errorMsgBuffer ? errorMsgBuffer : L"Unknown error.";                                                         \
        LocalFree(errorMsgBuffer);                                                                                                                   \
        while (!errorDescription.empty() && (errorDescription.back() == L'\n' || errorDescription.back() == L'\r')) {                                \
            errorDescription.pop_back();                                                                                                             \
        }                                                                                                                                            \
        return errorDescription;                                                                                                                     \
    }()

class Exception {
public:
    Exception(std::wstring file, std::wstring line, std::wstring column, std::wstring title = L"Exception", std::wstring note = L"")
        : file(std::move(file)), line(std::move(line)), column(std::move(column)), title(std::move(title)), note(std::move(note)) {
    }

    virtual ~Exception() = default;

    Exception(const Exception&) = delete;
    Exception& operator=(const Exception&) = delete;
    Exception(Exception&&) = delete;
    Exception& operator=(Exception&&) = delete;

    const constexpr std::wstring& GetFile() const {
        return file;
    }

    const constexpr std::wstring& GetLine() const {
        return line;
    }

    const constexpr std::wstring& GetColumn() const {
        return column;
    }

    const constexpr std::wstring& GetTitle() const {
        return title;
    }

    const constexpr std::wstring& GetNote() const {
        return note;
    }

    const constexpr std::wstring GetLocation() const {
        return std::wstring(L"Line [") + line + L"] in " + file;
    }

    const constexpr std::wstring GetFullMessage() const {
        return GetNote() + L"\nAt: " + GetLocation();
    }

private:
    std::wstring file;
    std::wstring line;
    std::wstring column;
    std::wstring title;
    std::wstring note;
};
