#include "GlobalId.hpp"

#include <iterator>
#include <type_traits>

#include "_formaterror.hpp"

GlobalId::GlobalId(
    std::string_view keyName,
    const std::uint32_t formId,
    const std::string& pluginName)
    : _keyName{keyName}
    , _formId{formId}
    , _pluginName{pluginName}
    , _form{nullptr}
{}

GlobalId
    GlobalId::constructFromToml(std::string_view keyName, toml::array& array)
{
    auto formIdValue = array[0].as_integer();
    auto pluginNameValue = array[1].as_string();

    if (formIdValue == nullptr) {
        throw std::runtime_error{
            formatError::missingOrInvalid("global variable ID", 0, "integer")};
    }

    if (pluginNameValue == nullptr) {
        throw std::runtime_error{
            formatError::missingOrInvalid("global variable ID", 1, "string")};
    }

    return GlobalId{
        keyName,
        static_cast<std::uint32_t>(formIdValue->get()),
        pluginNameValue->get()};
}
