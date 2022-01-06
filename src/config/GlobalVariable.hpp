#pragma once

#include <optional>
#include <string>

#include <toml++/toml.h>
#include <RE/F/FormTypes.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESDataHandler.h>

#include "../global.hpp"
#include "ConfigKey.hpp"
#include "FormId.hpp"
#include "Form.hpp"
#include "FormError.hpp"

namespace RE {
    class TESDataHandler;
    class TESGlobal;
} // namespace RE

template <typename KeyType>
class GlobalVariable : public Form<RE::TESGlobal> {
    const KeyType key_;
    const float defaultValue_;

public:
    explicit GlobalVariable(const KeyType key, const float defaultValue)
        : key_(key)
        , defaultValue_(defaultValue)
    {}

    KeyType key() const { return key_; }
    float value() const;
    bool valueAsBool() const { return value() != 0; }
    float defaultValue() const { return defaultValue_; }
};

template <typename KeyType>
inline float GlobalVariable<KeyType>::value() const
{
    using namespace std::literals;

    if (isFormLoaded()) {
        return form_->value;
    }

    LOG_INFO_FMT(
        "Form for {} not loaded. Returning default value."sv,
        toString(key()));
    return defaultValue_;
}
