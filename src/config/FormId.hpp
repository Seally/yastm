#pragma once

#include <exception>
#include <iterator>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_set>

#include <cstdint>

#include <boost/container_hash/hash.hpp>
#include <fmt/format.h>
#include <toml++/toml.h>

#include <RE/B/BSCoreTypes.h>

#include "../utilities/stringutils.hpp"

class FormId {
    RE::FormID id_;
    std::string pluginName_;

    mutable std::optional<std::string> cachedPluginNameLower_;
    mutable std::optional<std::size_t> cachedHashCode_;

    /**
     * @brief Lowercase-only version of the plugin name. For comparison and
     * hashing.
     **/
    const std::string& pluginNameLower_() const
    {
        if (cachedPluginNameLower_.has_value()) {
            return *cachedPluginNameLower_;
        }

        cachedPluginNameLower_ = getLowerString(pluginName_);

        return *cachedPluginNameLower_;
    }

    std::size_t calculateHashCode_() const
    {
        std::size_t seed = 0;

        boost::hash_combine(seed, id_);
        boost::hash_combine(seed, pluginNameLower_());

        return seed;
    }

    void invalidateCache_() const
    {
        cachedPluginNameLower_.reset();
        cachedHashCode_.reset();
    }

public:
    explicit FormId(const toml::array& arr);
    explicit FormId(RE::FormID id, std::string_view pluginName);

    FormId(const FormId&) = default;
    FormId(FormId&&) = default;
    FormId& operator=(const FormId&) = default;
    FormId& operator=(FormId&&) = default;

    RE::FormID id() const noexcept { return id_; }
    const std::string& pluginName() const noexcept { return pluginName_; }

    friend bool operator==(const FormId& lhs, const FormId& rhs) noexcept
    {
        return lhs.id_ == rhs.id_ &&
               lhs.pluginNameLower_() == rhs.pluginNameLower_();
    }

    std::size_t hash() const
    {
        if (cachedHashCode_.has_value()) {
            return *cachedHashCode_;
        }

        cachedHashCode_ = calculateHashCode_();

        return *cachedHashCode_;
    }
};

// Inject hash specialization into std namespace.
template <>
struct std::hash<FormId> {
    std::size_t operator()(const FormId& formId) const noexcept
    {
        return formId.hash();
    }
};

template <>
struct fmt::formatter<FormId> {
    constexpr auto parse(fmt::format_parse_context& ctx)
        -> decltype(ctx.begin())
    {
        // [ctx.begin(), ctx.end()) is a character range that contains a part of
        // the format string starting from the format specifications to be parsed,
        // e.g. in
        //
        //   fmt::format("{:f} - point of interest", point(1, 2));
        //
        // the range will contain "f} - point of interest". The formatter should
        // parse specifiers until '}' or the end of the range.

        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin();

        // Check if reached the end of the range:
        if (it != ctx.end() && *it != '}') {
            throw fmt::format_error("invalid format");
        }

        // Return an iterator past the end of the parsed range:
        return it;
    }

    template <typename FormatContext>
    auto format(const FormId& formId, FormatContext& ctx) -> decltype(ctx.out())
    {
        using namespace std::literals;

        return fmt::format_to(
            ctx.out(),
            "[{:#08x}, {}]"sv,
            formId.id(),
            formId.pluginName());
    }
};
