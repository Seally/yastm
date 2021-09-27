#ifndef LOADPRIORITY_HPP
#define LOADPRIORITY_HPP

#include <string>

enum class LoadPriority {
    /**
     * @brief Soul gem group are added to the list according to the group's soul
     * gem type.
     */
    Auto,
    /**
     * @brief These soul gem groups are added to the list first. When set to
     * AUTO, this is the load priority for reusable soul gems.
     */
    High,
    /**
     * @brief These soul gem groups are the second set added to the soul gem
     * map. When set to AUTO, this is the load priority for non-reusable
     * soul gems.
     */
    Normal,
    /**
     * @brief These soul gem groups are the last ones added to the soul gem map.
     * This priority level is only specified explicitly by the user.
     */
    Low,
    Invalid,
};

inline std::string_view toLoadPriorityString(const LoadPriority priority)
{
    switch (priority) {
    case LoadPriority::Auto:
        return "auto"sv;
    case LoadPriority::High:
        return "high"sv;
    case LoadPriority::Normal:
        return "normal"sv;
    case LoadPriority::Low:
        return "low"sv;
    }

    return "<invalid load priority>"sv;
}

inline LoadPriority fromLoadPriorityString(std::string_view str)
{
    if (str == "auto"sv) {
        return LoadPriority::Auto;
    }

    if (str == "high"sv) {
        return LoadPriority::High;
    }

    if (str == "normal"sv) {
        return LoadPriority::Normal;
    }

    if (str == "low"sv) {
        return LoadPriority::Low;
    }

    return LoadPriority::Invalid;
}

#endif // LOADPRIORITY_HPP
