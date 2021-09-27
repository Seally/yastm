#ifndef SOULGEMGROUP_HPP
#define SOULGEMGROUP_HPP

#include <memory>
#include <string>
#include <vector>

#include <toml++/toml.h>

#include "LoadPriority.hpp"
#include "SoulGemId.hpp"
#include "SoulSize.hpp"

class SoulGemGroup {
public:
    typedef std::string IdType;
    typedef std::vector<std::unique_ptr<SoulGemId>> MembersType;

private:
    const IdType _id;
    const bool _isReusable;
    const SoulSize _capacity;
    const LoadPriority _priority;
    MembersType _members;

public:
    template <typename iterator>
    explicit SoulGemGroup(
        const std::string& id,
        const bool isReusable,
        const SoulSize capacity,
        const LoadPriority priority,
        iterator memberBegin,
        iterator memberEnd);

    static SoulGemGroup constructFromToml(toml::table& table);

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

#endif // SOULGEMGROUP_HPP
