#pragma once

#include <exception>
#include <optional>
#include <string>

#include <toml++/toml.h>

#include "ConfigKey.hpp"
#include "FormId.hpp"

namespace RE {
    class TESDataHandler;
    class TESGlobal;
}

struct ParseContext;

class GlobalVariable {
    const ConfigKey _key;
    const float _defaultValue;
    std::optional<FormId> _formId;
    RE::TESGlobal* _form;

public:
    explicit GlobalVariable(const ConfigKey key, const float defaultValue)
        : _key{key}
        , _defaultValue{defaultValue}
        , _form{nullptr}
    {}

    void setFromToml(const toml::array& arr);
    void loadForm(RE::TESDataHandler* dataHandler);

    ConfigKey key() const { return _key; }
    float value() const;
    bool valueAsBool() const { return value() != 0; }
    float defaultValue() const { return _defaultValue; }

    const FormId& formId() const { return _formId.value(); }

    bool isConfigLoaded() const { return _formId.has_value(); }
    bool isFormLoaded() const { return _form != nullptr; }
};
