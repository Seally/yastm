#pragma once

#include <exception>
#include <memory>
#include <string>
#include <vector>

#include <RE/S/SoulLevels.h>
#include <toml++/toml_table.h>

#include "FormId.hpp"
#include "LoadPriority.hpp"
#include "SoulSize.hpp"

namespace RE {
    class TESDataHandler;
    class TESSoulGem;
} // end namespace RE

class SoulGemGroup {
public:
    using IdType = std::string;
    using MembersType = std::vector<FormId>;

private:
    IdType _id;
    bool _isReusable;
    SoulSize _capacity;
    LoadPriority _priority;
    MembersType _members;

public:
    explicit SoulGemGroup(const toml::table& table);

    const IdType& id() const { return _id; }
    bool isReusable() const { return _isReusable; }

    SoulSize capacity() const { return _capacity; }
    /**
     * @brief Returns the "effective" soul gem capacity, used to match against
     * the values reported by the game soul gem forms.
     */
    RE::SOUL_LEVEL effectiveCapacity() const { return toSoulLevel(capacity()); }

    LoadPriority rawPriority() const { return _priority; }
    LoadPriority priority() const
    {
        if (rawPriority() == LoadPriority::Auto) {
            return isReusable() ? LoadPriority::High : LoadPriority::Normal;
        }

        return rawPriority();
    }

    const MembersType& members() const { return _members; }
    const FormId& emptyMember() const { return members().front(); }
    const FormId& filledMember() const { return members().back(); }
};

class SoulGemGroupError : public std::runtime_error {
public:
    explicit SoulGemGroupError(const std::string& message);
};
