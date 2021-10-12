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
    FormId(const FormId& other) = default;

    std::uint32_t id() const { return _id; }
    const std::string& pluginName() const { return _pluginName; }

    template<typename iterator>
    static bool areAllUnique(iterator begin, iterator end);
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

template <typename iterator>
bool FormId::areAllUnique(iterator begin, iterator end) {
    static_assert(std::is_same_v<
        typename std::iterator_traits<iterator>::value_type,
        std::unique_ptr<FormId>>);

    // Hashing function that calculates the hash from the dereferenced value,
    // not the pointer value itself.
    struct Hash {
        std::size_t operator()(const FormId* const a) const noexcept {
            return std::hash<FormId>{}(*a);
        }
    };

    // Equality function that compares the dereferenced value, not the
    // pointer value itself.
    auto equals = [](const FormId* const a, const FormId* const b) {
        return *a == *b;
    };

    // Use an unordered set to disambiguate between FormIds. We override the
    // pointer's hashing function and equality comparator to compare the
    // dereferenced values, not the pointers themselves.
    std::unordered_set<const FormId*, Hash, decltype(equals)> uniques;

    std::size_t count = 0;

    for (auto it = begin; it != end; ++it) {
        uniques.insert(it->get());
        ++count;
    }

    return uniques.size() == count;
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

        return format_to(
            ctx.out(),
            "[{:#08x}, {}]"sv,
            formId.id(),
            formId.pluginName());
    }
};
