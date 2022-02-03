#pragma once

#include <algorithm>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>

#include <cctype>

inline char toUpperChar(const unsigned char input)
{
    return static_cast<char>(std::toupper(input));
}

inline char toLowerChar(const unsigned char input)
{
    return static_cast<char>(std::tolower(input));
}

inline void toUpperString(std::string& str)
{
    std::ranges::transform(str, str.begin(), toUpperChar);
}

inline void toLowerString(std::string& str)
{
    std::ranges::transform(str, str.begin(), toLowerChar);
}

inline std::string getUpperString(std::string_view str)
{
    std::string output;
    std::ranges::transform(str, std::back_inserter(output), toUpperChar);
    return output;
}

inline std::string getLowerString(std::string_view str)
{
    std::string output;
    std::ranges::transform(str, std::back_inserter(output), toLowerChar);
    return output;
}

inline void capitalizeFirstChar(std::string& str)
{
    if (str.size() > 0) {
        str.front() = toUpperChar(str.front());
    }
}

inline std::string getFirstCharCapitalizedString(std::string_view str)
{
    std::string output(str);
    capitalizeFirstChar(output);
    return output;
}

inline bool iequals(std::string_view lhs, std::string_view rhs)
{
    auto toLowerTransform(std::ranges::views::transform(toLowerChar));
    return std::ranges::equal(lhs | toLowerTransform, rhs | toLowerTransform);
}

inline void joinIfNotEmpty(
    std::string& dest,
    std::string_view source,
    std::string_view joinString = " ")
{
    if (source.size() > 0) {
        if (dest.size() > 0) {
            dest.append(joinString);
        }
        dest.append(source);
    }
};
