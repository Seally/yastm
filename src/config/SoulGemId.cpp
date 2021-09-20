#include "SoulGemId.hpp"

#include <iterator>
#include <type_traits>

#include "_formaterror.hpp"

SoulGemId::SoulGemId(const std::uint32_t formId, const std::string& pluginName)
    : _formId{formId}
    , _pluginName{pluginName}
{
}

SoulGemId SoulGemId::constructFromToml(toml::array& array) {
    auto formIdValue = array[0].as_integer();
    auto pluginNameValue = array[1].as_string();

    if (formIdValue == nullptr) {
        throw std::runtime_error{ formatError::missingOrInvalid("soul gem ID", 0, "integer") };
    }

    if (pluginNameValue == nullptr) {
        throw std::runtime_error{ formatError::missingOrInvalid("soul gem ID", 1, "string") };
    }

    return SoulGemId{ static_cast<std::uint32_t>(formIdValue->get()), pluginNameValue->get() };
}
