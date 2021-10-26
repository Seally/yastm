#pragma once

#include <exception>
#include <iterator>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <cstdint>

#include <boost/container_hash/hash.hpp>
#include <fmt/format.h>
#include <toml++/toml.h>

class FormId {
    std::uint32_t _id;
    std::string _pluginName;

public:
    explicit FormId(const toml::array& arr);
    explicit FormId(std::uint32_t id, std::string_view pluginName);
    FormId(const FormId&) = default;
    FormId(FormId&&) = default;

    std::uint32_t id() const { return _id; }
    const std::string& pluginName() const { return _pluginName; }
};

inline bool operator==(const FormId& lhs, const FormId& rhs) {
    return lhs.id() == rhs.id() && lhs.pluginName() == rhs.pluginName();
}

// Inject hash specialization into std namespace.
namespace std {
    template <>
    struct hash<FormId> {
        std::size_t operator()(const FormId& formId) const noexcept {
            std::size_t seed = 0;

            boost::hash_combine(seed, formId.id());
            boost::hash_combine(seed, formId.pluginName());

            return seed;
        }
    };
}

template <>
struct fmt::formatter<FormId> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        // [ctx.begin(), ctx.end()) is a character range that contains a part of
        // the format string starting from the format specifications to be parsed,
        // e.g. in
        //
        //   fmt::format("{:f} - point of interest", point{1, 2});
        //
        // the range will contain "f} - point of interest". The formatter should
        // parse specifiers until '}' or the end of the range.

        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin(), end = ctx.end();

        // Check if reached the end of the range:
        if (it != end && *it != '}') {
            throw format_error("invalid format");
        }

        // Return an iterator past the end of the parsed range:
        return it;
    }

    template <typename FormatContext>
    auto format(const FormId& formId, FormatContext& ctx)
        -> decltype(ctx.out()) {
        using namespace std::literals;

        return fmt::format_to(
            ctx.out(),
            "[{:#08x}, {}]"sv,
            formId.id(),
            formId.pluginName());
    }
};
