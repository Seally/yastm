#include "SoulGemGroup.hpp"

#include <concepts>
#include <iterator>
#include <queue>

#include <cassert>

#include "../global.hpp"
#include "../utilities/algorithms.hpp"
#include "ParseError.hpp"

using namespace std::literals;

namespace {
    std::string_view ID_KEY_("id");
    std::string_view ISREUSABLE_KEY_("isReusable");
    std::string_view CAPACITY_KEY_("capacity");
    std::string_view PRIORITY_KEY_("priority");
    std::string_view MEMBERS_KEY_("members");

    template <typename T>
    requires std::integral<T>
    constexpr SoulGemCapacity toSoulGemCapacityFromConfig_(const T capacity)
    {
        switch (capacity) {
        case 1:
            return SoulGemCapacity::Petty;
        case 2:
            return SoulGemCapacity::Lesser;
        case 3:
            return SoulGemCapacity::Common;
        case 4:
            return SoulGemCapacity::Greater;
        case 5:
            return SoulGemCapacity::Grand;
        case 6:
            return SoulGemCapacity::Black;
        }

        throw EntryValueOutOfRangeError(
            CAPACITY_KEY_.data(),
            EntryRange(1, 6),
            "Invalid value for entry 'capacity'");
    }

    std::size_t
        getExpectedMemberCountForCapacity_(const SoulGemCapacity capacity)
    {
        switch (capacity) {
        case SoulGemCapacity::Dual:
            return 6;
        case SoulGemCapacity::Black:
            return 2;
        }

        return capacity + 2;
    }

    std::string parseId_(const toml::table& table)
    {
        const auto value = table[ID_KEY_].as_string();

        if (value == nullptr) {
            throw InvalidEntryValueTypeError(
                ID_KEY_.data(),
                ValueType::String,
                "Expected string entry named 'id'");
        }

        return value->get();
    }

    SoulGemCapacity parseCapacity_(const toml::table& table)
    {
        const auto value = table[CAPACITY_KEY_].as_integer();

        if (value == nullptr) {
            throw InvalidEntryValueTypeError(
                CAPACITY_KEY_.data(),
                ValueType::Integer,
                "Expected integer entry named 'capacity'");
        }

        const auto capacity = toSoulGemCapacityFromConfig_(value->get());

        assert(capacity != SoulGemCapacity::Dual);

        return capacity;
    }

    bool parseIsReusable_(const toml::table& table)
    {
        return table[ISREUSABLE_KEY_].value_or(false);
    }

    LoadPriority parsePriority_(const toml::table& table)
    {
        const auto value = table[PRIORITY_KEY_].value_or("auto"sv);

        const LoadPriority priority = fromLoadPriorityString(value);

        if (priority == LoadPriority::Invalid) {
            throw EntryValueOutOfRangeError(
                PRIORITY_KEY_.data(),
                EntryRange{"auto", "low", "normal", "high"},
                "Invalid value for entry 'priority'");
        }

        return priority;
    }

    SoulGemGroup::MemberList
        parseMembers_(const toml::table& table, const SoulGemCapacity capacity)
    {
        const auto value = table[MEMBERS_KEY_].as_array();

        if (value == nullptr || value->empty()) {
            throw InvalidEntryValueTypeError(
                MEMBERS_KEY_.data(),
                ValueType::Array,
                "Expected non-empty array entry named 'members'");
        }

        SoulGemGroup::MemberList members;
        std::size_t index = 0;

        for (const toml::node& elem : *value) {
            try {
                elem.visit([&](auto&& el) {
                    if constexpr (toml::is_array<decltype(el)>) {
                        members.emplace_back(el);
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

        if (!areAllUnique(members.cbegin(), members.cend())) {
            throw ArrayDuplicateEntriesError(
                MEMBERS_KEY_.data(),
                "Duplicate values in 'members' array");
        }

        if (const auto expectedMemberCount =
                getExpectedMemberCountForCapacity_(capacity);
            expectedMemberCount != members.size()) {
            throw ArrayInvalidSizeError(
                MEMBERS_KEY_.data(),
                EntryRange(static_cast<int>(expectedMemberCount)),
                "Invalid number of members in 'members' array");
        }

        return members;
    }
} // namespace

SoulGemGroup::SoulGemGroup(const toml::table& table)
{
    // The nested error includes the ID value, which isn't present if this
    // fails, so we do this before the try...catch.
    id_ = parseId_(table);

    try {
        capacity_ = parseCapacity_(table);
        isReusable_ = parseIsReusable_(table);
        priority_ = parsePriority_(table);
        members_ = parseMembers_(table, capacity_);
    } catch (...) {
        std::throw_with_nested(SoulGemGroupError(fmt::format(
            FMT_STRING("Error while parsing soul gem group \"{}\":"sv),
            id_)));
    }
}
