#ifndef GLOBALID_HPP
#define GLOBALID_HPP

#include <cstdint>
#include <string>

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

#endif // GLOBALID_HPP
