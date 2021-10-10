#include "SoulGemGroup.hpp"

#include <queue>

#include "../global.hpp"
#include "ParseError.hpp"

SoulGemGroup::SoulGemGroup(const toml::table& table)
{
    using namespace std::literals;

    std::string_view ID_KEY{"id"};
    std::string_view ISREUSABLE_KEY{"isReusable"};
    std::string_view CAPACITY_KEY{"capacity"};
    std::string_view PRIORITY_KEY{"priority"};
    std::string_view MEMBERS_KEY{"members"};

    auto idValue = table[ID_KEY].as_string();
    bool isReusable = table[ISREUSABLE_KEY].value_or(false);
    auto capacityValue = table[CAPACITY_KEY].as_integer();
    auto priorityStr = table[PRIORITY_KEY].value_or("auto"sv);
    auto membersValue = table[MEMBERS_KEY].as_array();

    if (idValue == nullptr) {
        throw InvalidEntryValueTypeError{
            ID_KEY.data(),
            ValueType::String,
            "Expected string entry named 'id'"};
    }

    const auto id = idValue->get();

    try {
        if (capacityValue == nullptr) {
            throw InvalidEntryValueTypeError{
                CAPACITY_KEY.data(),
                ValueType::Integer,
                "Expected integer entry named 'capacity'"};
        }

        if (membersValue == nullptr || membersValue->empty()) {
            throw InvalidEntryValueTypeError{
                MEMBERS_KEY.data(),
                ValueType::Array,
                "Expected non-empty array entry named 'members'"};
        }

        MembersType members;
        std::size_t index = 0;

        for (const toml::node& elem : *membersValue) {
            try {
                elem.visit([&](auto&& el) {
                    if constexpr (toml::is_array<decltype(el)>) {
                        members.emplace_back(new FormId{el});
                    } else {
                        throw EntryError(
                            index,
                            fmt::format(
                                FMT_STRING("members[{}] is not an array"),
                                index));
                    }
                });
            } catch (...) {
                std::throw_with_nested(EntryError(
                    index,
                    fmt::format(
                        FMT_STRING("Invalid form ID entry at members[{}]"sv),
                        index)));
            }
            ++index;
        }

        const auto capacity = capacityValue->get();

        if (!isValidSoulCapacity(capacity)) {
            throw EntryValueOutOfRangeError(
                CAPACITY_KEY.data(),
                EntryRange(
                    static_cast<int>(SoulSize::Petty),
                    static_cast<int>(SoulSize::Black)),
                "Invalid value for entry 'capacity'");
        }

        const SoulSize soulCapacity = static_cast<SoulSize>(capacity);

        if (!FormId::areAllUnique(members.cbegin(), members.cend())) {
            throw ArrayDuplicateEntriesError(
                MEMBERS_KEY.data(),
                "Duplicate values in 'members' array");
        }

        const LoadPriority priority = fromLoadPriorityString(priorityStr);

        if (priority == LoadPriority::Invalid) {
            throw EntryValueOutOfRangeError(
                PRIORITY_KEY.data(),
                EntryRange{"auto", "low", "normal", "high"},
                "Invalid value for entry 'priority'");
        }

        if (const auto expectedVariantCount =
                getVariantCountForCapacity(soulCapacity);
            expectedVariantCount != members.size()) {
            throw ArrayInvalidSizeError{
                MEMBERS_KEY.data(),
                EntryRange(static_cast<int>(expectedVariantCount)),
                "Invalid number of members in 'members' array"};
        }

        _id = id;
        _isReusable = isReusable;
        _capacity = soulCapacity;
        _priority = priority;
        _members = std::move(members);
    } catch (...) {
        std::throw_with_nested(SoulGemGroupError(
            id,
            fmt::format(
                FMT_STRING("Error while parsing soul gem group \"{}\":"sv),
                id)));
    }
}

SoulGemGroupError::SoulGemGroupError(
    std::string_view id,
    const std::string& message)
    : std::runtime_error{message}
    , id{id}
{}
