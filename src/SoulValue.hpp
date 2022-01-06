#pragma once

#include <compare>
#include <concepts>
#include <exception>
#include <type_traits>

#include <fmt/format.h>

#include "config/SoulSize.hpp"

class SoulGemCapacityValue;

class SoulSizeValue {
public:
    using ValueType = SoulSize;
    using UnderlyingValueType = std::underlying_type_t<ValueType>;

private:
    UnderlyingValueType value_;

    constexpr SoulSizeValue(const UnderlyingValueType soulSize)
        : value_(soulSize)
    {}

public:
    constexpr SoulSizeValue(const ValueType soulSize)
        : value_(static_cast<UnderlyingValueType>(soulSize))
    {}

    constexpr UnderlyingValueType raw() const { return value_; }

    constexpr SoulSizeValue& operator=(const ValueType soulSize)
    {
        value_ = static_cast<UnderlyingValueType>(soulSize);

        return *this;
    }

    constexpr operator ValueType() const
    {
#define CONV_CASE(Value)                                     \
    case static_cast<UnderlyingValueType>(ValueType::Value): \
        return ValueType::Value

        switch (value_) {
            CONV_CASE(None);
            CONV_CASE(Petty);
            CONV_CASE(Lesser);
            CONV_CASE(Common);
            CONV_CASE(Greater);
            CONV_CASE(Grand);
            CONV_CASE(Black);
        }
#undef CONV_CASE

        throw std::runtime_error(
            fmt::format("Cannot convert {} to SoulSize", value_));
    }

    constexpr bool isValid() const
    {
        return static_cast<UnderlyingValueType>(ValueType::First) <= value_ &&
               value_ <= static_cast<UnderlyingValueType>(ValueType::Last);
    }

    constexpr SoulSizeValue& operator++()
    {
        value_ += 1;
        return *this;
    }

    constexpr SoulSizeValue operator++(int)
    {
        SoulSizeValue tmp = *this;
        value_ += 1;
        return tmp;
    }

    constexpr SoulSizeValue& operator--()
    {
        value_ -= 1;
        return *this;
    }

    constexpr SoulSizeValue operator--(int)
    {
        SoulSizeValue tmp = *this;
        value_ -= 1;
        return tmp;
    }

    constexpr std::strong_ordering operator<=>(const SoulSize soulSize)
    {
        return value_ <=>
               static_cast<SoulSizeValue::UnderlyingValueType>(soulSize);
    }

    constexpr std::strong_ordering operator<=>(const SoulSizeValue& other)
    {
        return value_ <=> other.value_;
    }

    friend constexpr std::strong_ordering operator<=>(
        const SoulSizeValue& soulSize,
        const SoulGemCapacityValue& capacity);
    friend constexpr bool operator==(
        const SoulSizeValue& soulSize,
        const SoulGemCapacityValue& capacity);
};

class SoulGemCapacityValue {
public:
    using ValueType = SoulGemCapacity;
    using UnderlyingValueType = std::underlying_type_t<ValueType>;

private:
    UnderlyingValueType value_;

public:
    constexpr SoulGemCapacityValue(const UnderlyingValueType capacity)
        : value_(capacity)
    {}

    constexpr SoulGemCapacityValue(const ValueType capacity)
        : SoulGemCapacityValue(static_cast<UnderlyingValueType>(capacity))
    {}

    constexpr UnderlyingValueType raw() const { return value_; }

    constexpr SoulGemCapacityValue& operator=(const ValueType capacity)
    {
        value_ = static_cast<UnderlyingValueType>(capacity);

        return *this;
    }

    constexpr operator ValueType() const
    {
#define CONV_CASE(Value)                                     \
    case static_cast<UnderlyingValueType>(ValueType::Value): \
        return ValueType::Value

        switch (value_) {
            CONV_CASE(Petty);
            CONV_CASE(Lesser);
            CONV_CASE(Common);
            CONV_CASE(Greater);
            CONV_CASE(Grand);
            CONV_CASE(Dual);
            CONV_CASE(Black);
        }
#undef CONV_CASE

        throw std::runtime_error(
            fmt::format("Cannot convert {} to SoulGemCapacity", value_));
    }

    constexpr bool isValid() const
    {
        return static_cast<UnderlyingValueType>(ValueType::First) <= value_ &&
               value_ <= static_cast<UnderlyingValueType>(ValueType::Last);
    }

    constexpr SoulGemCapacityValue& operator++()
    {
        value_ += 1;
        return *this;
    }

    constexpr SoulGemCapacityValue operator++(int)
    {
        SoulGemCapacityValue tmp = *this;
        value_ += 1;
        return tmp;
    }

    constexpr SoulGemCapacityValue& operator--()
    {
        value_ -= 1;
        return *this;
    }

    constexpr SoulGemCapacityValue operator--(int)
    {
        SoulGemCapacityValue tmp = *this;
        value_ -= 1;
        return tmp;
    }

    constexpr std::strong_ordering operator<=>(const SoulGemCapacity capacity)
    {
        return value_ <=> static_cast<UnderlyingValueType>(capacity);
    }

    constexpr std::strong_ordering
        operator<=>(const SoulGemCapacityValue& other)
    {
        return value_ <=> other.value_;
    }

    friend constexpr std::strong_ordering operator<=>(
        const SoulSizeValue& soulSize,
        const SoulGemCapacityValue& capacity);
    friend constexpr bool operator==(
        const SoulSizeValue& soulSize,
        const SoulGemCapacityValue& capacity);
};

constexpr std::strong_ordering operator<=>(
    const SoulSizeValue& soulSize,
    const SoulGemCapacityValue& capacity)
{
    // Same line means equivalent:
    //
    // SoulSize   Capacity
    //
    // None=0
    // Petty=1    Petty=0
    // Lesser=2   Lesser=1
    // Common=3   Common=2
    // Greater=4  Greater=3
    // Grand=5    Grand=4, Dual=5
    // Black=6    Black=6
    if (capacity >= SoulGemCapacity::Dual) {
        return soulSize.value_ <=> capacity.value_;
    }

    return soulSize.value_ <=> capacity.value_ + 1;
}

constexpr bool operator==(
    const SoulSizeValue& soulSize,
    const SoulGemCapacityValue& capacity)
{
    // Same line means equivalent:
    //
    // SoulSize   Capacity
    //
    // None=0
    // Petty=1    Petty=0
    // Lesser=2   Lesser=1
    // Common=3   Common=2
    // Greater=4  Greater=3
    // Grand=5    Grand=4, Dual=5
    // Black=6    Black=6
    if (capacity >= SoulGemCapacity::Dual) {
        return soulSize.value_ == capacity.value_;
    }

    return soulSize.value_ == capacity.value_ + 1;
}

template <>
struct fmt::formatter<SoulSizeValue> : fmt::formatter<SoulSize> {
    template <typename FormatContext>
    auto format(const SoulSizeValue& soulSize, FormatContext& ctx)
        -> decltype(ctx.out())
    {
        return fmt::formatter<SoulSize>::format(
            static_cast<SoulSize>(soulSize),
            ctx);
    }
};

template <>
struct fmt::formatter<SoulGemCapacityValue> : fmt::formatter<SoulGemCapacity> {
    template <typename FormatContext>
    auto format(const SoulGemCapacityValue& capacity, FormatContext& ctx)
        -> decltype(ctx.out())
    {
        return fmt::formatter<SoulGemCapacity>::format(
            static_cast<SoulGemCapacity>(capacity),
            ctx);
    }
};
