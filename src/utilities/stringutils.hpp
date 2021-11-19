#pragma once

#include <algorithm>
#include <string>
#include <cctype>

inline char toUpper(const unsigned char input)
{
    return static_cast<char>(std::toupper(input));
}

inline void toUpperString(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), toUpper);
}

inline void toUpperString(const std::string& source, std::string& target)
{
    target.resize(source.size());
    std::transform(source.begin(), source.end(), target.begin(), toUpper);
}

inline char toLower(const unsigned char input)
{
    return static_cast<char>(std::tolower(input));
}

inline void toLowerString(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), toLower);
}

inline void toLowerString(const std::string& source, std::string& target)
{
    target.resize(source.size());
    std::transform(source.begin(), source.end(), target.begin(), toLower);
}

inline void capitalizeFirstChar(const std::string& source, std::string& target)
{
    target.front() = toUpper(source.front());
}
