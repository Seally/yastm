#pragma once

#include <exception>
#include <optional>
#include <string>

#include <toml++/toml.h>

#include "FormId.hpp"

namespace RE {
    class TESDataHandler;
    class TESGlobal;
}

struct ParseContext;

class GlobalVariable {
    const float _defaultValue;
    std::optional<FormId> _formId;
    RE::TESGlobal* _form;

public:
    explicit GlobalVariable(const float defaultValue) : _defaultValue{defaultValue}, _form{nullptr} {}

    void setFromToml(const toml::array& arr);
    void loadForm(RE::TESDataHandler* dataHandler);

    float value() const;
    bool valueAsBool() const { return value() != 0; }
    float defaultValue() const { return _defaultValue; }

    const FormId& formId() const { return _formId.value(); }

    bool isConfigLoaded() const { return _formId.has_value(); }
    bool isFormLoaded() const { return _form != nullptr; }
};
