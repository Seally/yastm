#pragma once

#include <bit>
#include <optional>

#include <cstdint>

enum class Instruction {
    JMP,
    CALL,
};

template <Instruction, std::uint8_t>
struct InstructionData;

namespace internal {
    template <std::uint8_t InstructionCode>
    inline std::optional<std::int32_t>
        getRawOffset_(const std::uintptr_t address)
    {
        const std::uint8_t* const ptr =
            reinterpret_cast<std::uint8_t*>(address);

        if (ptr[0] != InstructionCode) {
            return std::nullopt;
        }

        // Exploit the shared endianness of this and the argument value.
        union {
            std::int32_t value;
            std::uint8_t byte[4];
        } offset;

        offset.byte[0] = ptr[1];
        offset.byte[1] = ptr[2];
        offset.byte[2] = ptr[3];
        offset.byte[3] = ptr[4];

        return offset.value;
    }
} // namespace internal

template <>
struct InstructionData<Instruction::CALL, 0xe8> {
    static constexpr std::size_t size = 5;

    static std::optional<std::int32_t> arg0(const std::uintptr_t address)
    {
        return internal::getRawOffset_<0xe8>(address);
    }

    static constexpr auto rawOffset = InstructionData::arg0;

    static std::optional<std::ptrdiff_t> offset(const std::uintptr_t address)
    {
        if (const auto rawOffset_ = rawOffset(address);
            rawOffset_.has_value()) {
            return *rawOffset_ + size;
        }

        return std::nullopt;
    }

    static std::optional<std::uintptr_t>
        targetAddress(const std::uintptr_t address)
    {
        if (const auto offset_ = offset(address); offset_.has_value()) {
            return address + *offset_;
        }

        return std::nullopt;
    }
};

template <>
struct InstructionData<Instruction::JMP, 0xe9> {
    static constexpr std::size_t size = 5;

    static std::optional<std::int32_t> arg0(const std::uintptr_t address)
    {
        return internal::getRawOffset_<0xe9>(address);
    }

    static constexpr auto rawOffset = InstructionData::arg0;

    static std::optional<std::ptrdiff_t> offset(const std::uintptr_t address)
    {
        if (const auto rawOffset_ = rawOffset(address);
            rawOffset_.has_value()) {
            return *rawOffset_ + size;
        }

        return std::nullopt;
    }

    static std::optional<std::uintptr_t>
        targetAddress(const std::uintptr_t address)
    {
        if (const auto offset_ = offset(address); offset_.has_value()) {
            return address + *offset_;
        }

        return std::nullopt;
    }
};
