#include "FormId.hpp"

#include "ParseError.hpp"

FormId::FormId(const toml::array& arr)
{
    auto formIdValue = arr[0].as_integer();
    auto pluginNameValue = arr[1].as_string();

    if (formIdValue == nullptr) {
        throw ParseError("Form ID is missing or invalid");
    }

    if (pluginNameValue == nullptr) {
        throw ParseError("Plugin name is missing or invalid");
    }

    id_ = static_cast<RE::FormID>(formIdValue->get());
    pluginName_ = pluginNameValue->get();
}

FormId::FormId(const RE::FormID id, std::string_view pluginName)
    : id_(id)
    , pluginName_(pluginName)
{}
