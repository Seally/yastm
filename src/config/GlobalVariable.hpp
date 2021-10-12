#pragma once

#include <optional>
#include <string>

#include <toml++/toml.h>

#include "ConfigKey.hpp"
#include "FormId.hpp"
#include "Form.hpp"

namespace RE {
    class TESDataHandler;
    class TESGlobal;
}

class GlobalVariable : public Form<RE::TESGlobal> {
    const BoolConfigKey _key;
    const float _defaultValue;

public:
    explicit GlobalVariable(const BoolConfigKey key, const float defaultValue)
        : _key{key}
        , _defaultValue{defaultValue}
    {}

    BoolConfigKey key() const { return _key; }
    float value() const;
    bool valueAsBool() const { return value() != 0; }
    float defaultValue() const { return _defaultValue; }
};
