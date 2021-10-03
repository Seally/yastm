#ifndef CONFIG_GLOBALID_HPP
#define CONFIG_GLOBALID_HPP

#include <cstdint>
#include <string>

#include <fmt/format.h>
#include <toml++/toml.h>

namespace RE {
    class TESGlobal;
}

class GlobalId {
    std::string _keyName;
    std::uint32_t _formId;
    std::string _pluginName;
    RE::TESGlobal* _form;

public:
    explicit GlobalId(
        std::string_view keyName,
        const std::uint32_t formId,
        const std::string& pluginName);
    GlobalId(const GlobalId& other)
        : _keyName{other._keyName}
        , _formId{other._formId}
        , _pluginName{other._pluginName}
        , _form{other._form}
    {}

    static GlobalId
        constructFromToml(std::string_view keyName, toml::array& array);

    const std::string& keyName() const { return _keyName; }
    std::uint32_t formId() const { return _formId; }
    const std::string& pluginName() const { return _pluginName; }

    void setForm(RE::TESGlobal* const form) { _form = form; }
    RE::TESGlobal* form() const { return _form; }

    GlobalId& operator=(const GlobalId& other)
    {
        _keyName = other._keyName;
        _formId = other._formId;
        _pluginName = other._pluginName;
        _form = other._form;

        return *this;
    }
};

template <>
struct fmt::formatter<GlobalId> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
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
    auto format(const GlobalId& globalId, FormatContext& ctx)
        -> decltype(ctx.out())
    {
        using namespace std::literals;

        return format_to(
            ctx.out(),
            "[ID:{:08x}] in {}"sv,
            globalId.formId(),
            globalId.pluginName());
    }
};

#endif // CONFIG_GLOBALID_HPP
