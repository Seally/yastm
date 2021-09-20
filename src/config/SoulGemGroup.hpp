#ifndef SOULGEMGROUP_HPP
#define SOULGEMGROUP_HPP

#include <string>
#include <memory>
#include <vector>

#include <toml++/toml.h>

#include "SoulGemId.hpp"
#include "SoulSize.hpp"

class SoulGemGroup {
    const std::string _id;
    const bool _isReusable;
    const SoulSize _capacity;
    std::vector<std::shared_ptr<SoulGemId>> _members;

public:
    template<typename iterator>
    explicit SoulGemGroup(const std::string& id, const bool isReusable, const SoulSize capacity, iterator memberBegin, iterator memberEnd);

    static SoulGemGroup constructFromToml(toml::table& table);

    const std::string& id() const { return _id; }
    bool isReusable() const { return _isReusable; }
    SoulSize capacity() const { return _capacity; }
    /**
     * @brief Returns the effective soul gem capacity.
     *
     * This function exists because the game does not distinguish black souls and
     * white grand souls except by checking a record flag for soul trap eligibility.
     *
     * This function will return SoulSize::Grand for gems that can hold black souls.
     */
    SoulSize effectiveCapacity() const {
        return capacity() == SoulSize::Black ? SoulSize::Grand : capacity();
    }
    const std::vector<std::shared_ptr<SoulGemId>>& members() const { return _members; }
};

#endif // SOULGEMGROUP_HPP
