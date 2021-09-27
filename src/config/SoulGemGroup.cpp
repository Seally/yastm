#include "SoulGemGroup.hpp"

#include "_formaterror.hpp"

template <typename iterator>
inline SoulGemGroup::SoulGemGroup(
    const std::string& id,
    const bool isReusable,
    const SoulSize capacity,
    const LoadPriority priority,
    iterator memberBegin,
    iterator memberEnd)
    : _id{id}
    , _isReusable{isReusable}
    , _capacity{capacity}
    , _priority{priority}
{
    for (auto it = memberBegin; it != memberEnd; ++it) {
        _members.push_back(std::make_unique<SoulGemId>(*it));
    }
}

SoulGemGroup SoulGemGroup::constructFromToml(toml::table& table)
{
    using namespace std::literals;

    auto idValue = table["id"sv].as_string();
    bool isReusable = table["isReusable"sv].value_or(false);
    auto capacityValue = table["capacity"sv].as_integer();
    auto priorityStr = table["priority"sv].value_or("auto"sv);
    auto membersValue = table["members"sv].as_array();

    if (idValue == nullptr) {
        throw std::runtime_error{formatError::missingOrInvalid(
            "soul gem group"sv,
            "id"sv,
            "string"sv)};
    }

    if (capacityValue == nullptr) {
        throw std::runtime_error{formatError::missingOrInvalid(
            "soul gem group"sv,
            "capacity"sv,
            "integer"sv)};
    }

    if (membersValue == nullptr || membersValue->empty()) {
        throw std::runtime_error{std::format(
            "Soul gem group '{}' does not have any members."sv,
            idValue->get())};
    }

    std::vector<SoulGemId> members;

    for (toml::node& elem : *membersValue) {
        elem.visit([&](auto&& el) {
            if constexpr (toml::is_array<decltype(el)>) {
                members.push_back(SoulGemId::constructFromToml(el));
            } else {
                throw std::runtime_error{
                    "Invalid member type in members array."};
            }
        });
    }

    if (!SoulGemId::areAllUnique(members.cbegin(), members.cend())) {
        throw std::runtime_error{std::format(
            "Soul gem group '{}' contains duplicate members."sv,
            idValue->get())};
    }

    SoulSize capacity = static_cast<SoulSize>(capacityValue->get());

    if (!isValidSoulCapacity(capacity)) {
        throw std::runtime_error{std::format(
            "Soul gem group '{}' has invalid capacity {}"sv,
            idValue->get(),
            static_cast<int>(capacity))};
    }

    const LoadPriority priority = fromLoadPriorityString(priorityStr);

    if (priority == LoadPriority::Invalid) {
        throw std::runtime_error{std::format(
            "Soul gem group '{}' has invalid priority {}.",
            idValue->get(),
            static_cast<int>(capacity),
            priorityStr)};
    }

    if (const auto expectedVariantCount = getVariantCountForCapacity(capacity);
        expectedVariantCount != members.size()) {
        throw std::runtime_error{std::format(
            "Soul gem group '{}' has capacity {} and must have {} members.",
            idValue->get(),
            static_cast<int>(capacity),
            expectedVariantCount)};
    }

    return SoulGemGroup{
        idValue->get(),
        isReusable,
        capacity,
        priority,
        members.cbegin(),
        members.cend()};
}
