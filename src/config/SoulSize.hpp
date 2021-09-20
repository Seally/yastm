#ifndef SOULSIZE_HPP
#define SOULSIZE_HPP

#include <type_traits>

// Note: We're using unscoped enums for this since we need to do a LOT of
//       comparisons with integers and I don't want to write dozens of
//       operator overloads for this.
enum SoulSize {
    None = 0,
    Petty = 1,
    Lesser = 2,
    Common = 3,
    Greater = 4,
    Grand = 5,
    Black = 6,
};

template<typename T>
inline SoulSize toSoulSize(const T value) {
    static_assert(std::is_integral_v<T>);

    return static_cast<SoulSize>(value);
}

inline bool isValidSoulCapacity(const SoulSize soulCapacity) {
    return SoulSize::Petty <= soulCapacity && soulCapacity <= SoulSize::Black;
}

inline bool isValidContainedSoulSize(const SoulSize soulCapacity, const SoulSize containedSoulSize) {
    if (soulCapacity == SoulSize::Black) {
        return containedSoulSize == SoulSize::None || containedSoulSize == SoulSize::Black;
    }

    return SoulSize::None <= containedSoulSize && containedSoulSize <= soulCapacity;
}

inline std::size_t getVariantCountForCapacity(const SoulSize soulCapacity) {
    // Black soul gems only need 2 variants: filled and unfilled.
    if (soulCapacity == SoulSize::Black) {
        return 2;
    }

    return static_cast<std::size_t>(soulCapacity) + 1;
}

#endif // SOULSIZE_HPP
