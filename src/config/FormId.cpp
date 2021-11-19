#include "FormId.hpp"

#include "../utilities/stringutils.hpp"

#include "ParseError.hpp"

FormId::FormId(const toml::array& arr)
{
    auto formIdValue = arr[0].as_integer();
    auto pluginNameValue = arr[1].as_string();

    if (formIdValue == nullptr) {
        throw InvalidEntryValueTypeError{
            static_cast<std::size_t>(0),
            ValueType::Integer,
            "Form ID is missing or invalid"};
    }

    if (pluginNameValue == nullptr) {
        throw InvalidEntryValueTypeError{
            static_cast<std::size_t>(1),
            ValueType::String,
            "Plugin name is missing or invalid"};
    }

    _id = static_cast<std::uint32_t>(formIdValue->get());
    _pluginName = pluginNameValue->get();
    toLowerString(_pluginName, _pluginNameLower);
}

FormId::FormId(const std::uint32_t id, std::string_view pluginName)
    : _id{id}
    , _pluginName{pluginName}
{
    toLowerString(_pluginName, _pluginNameLower);
}
