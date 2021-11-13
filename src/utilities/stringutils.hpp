#pragma once

#include <cctype>
#include <string>

inline char toUpper(const char input)
{
    return static_cast<char>(std::toupper(static_cast<unsigned char>(input)));
}

inline char toLower(const char input)
{
    return static_cast<char>(std::tolower(static_cast<unsigned char>(input)));
}

inline void capitalizeFirstChar(std::string& str)
{
    str.front() = toUpper(str.front());
}
