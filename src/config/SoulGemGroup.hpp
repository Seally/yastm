#pragma once

#include <exception>
#include <memory>
#include <string>
#include <vector>

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
    typedef std::string IdType;
    typedef std::vector<std::unique_ptr<FormId>> MembersType;

private:
    IdType _id;
    bool _isReusable;
    SoulSize _capacity;
    LoadPriority _priority;
    MembersType _members;

public:
    explicit SoulGemGroup(const toml::table& table);

    const std::string& id() const { return _id; }
    bool isReusable() const { return _isReusable; }

    SoulSize capacity() const { return _capacity; }
    /**
     * @brief Returns the effective soul gem capacity.
     *
     * This function exists because the game does not distinguish black souls
     * and white grand souls except by checking a record flag for soul trap
     * eligibility.
     *
     * This function will return SoulSize::Grand for gems that can hold black
     * souls.
     */
    SoulSize effectiveCapacity() const
    {
        return capacity() == SoulSize::Black ? SoulSize::Grand : capacity();
    }

    LoadPriority rawPriority() const { return _priority; }
    LoadPriority priority() const
    {
        if (rawPriority() == LoadPriority::Auto) {
            return isReusable() ? LoadPriority::High : LoadPriority::Normal;
        }

        return LoadPriority::Low;
    }

    const MembersType& members() const { return _members; }
};

class SoulGemGroupError : public std::runtime_error {
public:
    const std::string id;

    explicit SoulGemGroupError(std::string_view id, const std::string& message);
};
