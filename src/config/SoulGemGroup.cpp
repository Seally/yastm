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
    std::string_view ID_KEY{"id"};
    std::string_view ISREUSABLE_KEY{"isReusable"};
    std::string_view CAPACITY_KEY{"capacity"};
    std::string_view PRIORITY_KEY{"priority"};
    std::string_view MEMBERS_KEY{"members"};
} // end anonymous namespace

template <typename T>
requires std::integral<T> constexpr SoulGemCapacity
    _toSoulGemCapacityFromConfig(const T capacity)
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
        CAPACITY_KEY.data(),
        EntryRange(1, 6),
        "Invalid value for entry 'capacity'");
}

std::size_t _getExpectedMemberCountForCapacity(const SoulGemCapacity capacity)
{
    switch (capacity) {
    case SoulGemCapacity::Dual:
        return 6;
    case SoulGemCapacity::Black:
        return 2;
    }

    return capacity + 2;
}

std::string _parseId(const toml::table& table)
{
    const auto value = table[ID_KEY].as_string();

    if (value == nullptr) {
        throw InvalidEntryValueTypeError{
            ID_KEY.data(),
            ValueType::String,
            "Expected string entry named 'id'"};
    }

    return value->get();
}

SoulGemCapacity _parseCapacity(const toml::table& table)
{
    const auto value = table[CAPACITY_KEY].as_integer();

    if (value == nullptr) {
        throw InvalidEntryValueTypeError{
            CAPACITY_KEY.data(),
            ValueType::Integer,
            "Expected integer entry named 'capacity'"};
    }

    const auto capacity = _toSoulGemCapacityFromConfig(value->get());

    assert(capacity != SoulGemCapacity::Dual);

    return capacity;
}

bool _parseIsReusable(const toml::table& table)
{
    return table[ISREUSABLE_KEY].value_or(false);
}

LoadPriority _parsePriority(const toml::table& table)
{
    const auto value = table[PRIORITY_KEY].value_or("auto"sv);

    const LoadPriority priority = fromLoadPriorityString(value);

    if (priority == LoadPriority::Invalid) {
        throw EntryValueOutOfRangeError(
            PRIORITY_KEY.data(),
            EntryRange{"auto", "low", "normal", "high"},
            "Invalid value for entry 'priority'");
    }

    return priority;
}

SoulGemGroup::MemberList
    _parseMembers(const toml::table& table, const SoulGemCapacity capacity)
{
    const auto value = table[MEMBERS_KEY].as_array();

    if (value == nullptr || value->empty()) {
        throw InvalidEntryValueTypeError{
            MEMBERS_KEY.data(),
            ValueType::Array,
            "Expected non-empty array entry named 'members'"};
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
            MEMBERS_KEY.data(),
            "Duplicate values in 'members' array");
    }

    if (const auto expectedMemberCount =
            _getExpectedMemberCountForCapacity(capacity);
        expectedMemberCount != members.size()) {
        throw ArrayInvalidSizeError{
            MEMBERS_KEY.data(),
            EntryRange(static_cast<int>(expectedMemberCount)),
            "Invalid number of members in 'members' array"};
    }

    return members;
}

SoulGemGroup::SoulGemGroup(const toml::table& table)
{
    // The nested error includes the ID value, which isn't present if this
    // fails, so we do this before the try...catch.
    _id = _parseId(table);

    try {
        _capacity = _parseCapacity(table);
        _isReusable = _parseIsReusable(table);
        _priority = _parsePriority(table);
        _members = _parseMembers(table, _capacity);
    } catch (...) {
        std::throw_with_nested(SoulGemGroupError(fmt::format(
            FMT_STRING("Error while parsing soul gem group \"{}\":"sv),
            _id)));
    }
}

SoulGemGroupError::SoulGemGroupError(const std::string& message)
    : std::runtime_error{message}
{}
