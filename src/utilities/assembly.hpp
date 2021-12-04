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
    std::optional<std::int32_t> _getRawOffset(const std::uintptr_t address)
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
}

template <>
struct InstructionData<Instruction::CALL, 0xe8> {
    static constexpr std::size_t size = 5;

    static std::optional<std::int32_t> arg0(const std::uintptr_t address)
    {
        return internal::_getRawOffset<0xe8>(address);
    }

    static constexpr auto rawOffset = InstructionData::arg0;

    static std::optional<std::ptrdiff_t> offset(const std::uintptr_t address)
    {
        if (const auto _rawOffset = rawOffset(address);
            _rawOffset.has_value()) {
            return *_rawOffset + size;
        }

        return std::nullopt;
    }

    static std::optional<std::uintptr_t>
        targetAddress(const std::uintptr_t address)
    {
        if (const auto _offset = offset(address); _offset.has_value()) {
            return address + *_offset;
        }

        return std::nullopt;
    }
};

template <>
struct InstructionData<Instruction::JMP, 0xe9> {
    static constexpr std::size_t size = 5;

    static std::optional<std::int32_t> arg0(const std::uintptr_t address)
    {
        return internal::_getRawOffset<0xe9>(address);
    }

    static constexpr auto rawOffset = InstructionData::arg0;

    static std::optional<std::ptrdiff_t> offset(const std::uintptr_t address)
    {
        if (const auto _rawOffset = rawOffset(address);
            _rawOffset.has_value()) {
            return *_rawOffset + size;
        }

        return std::nullopt;
    }

    static std::optional<std::uintptr_t>
        targetAddress(const std::uintptr_t address)
    {
        if (const auto _offset = offset(address); _offset.has_value()) {
            return address + *_offset;
        }

        return std::nullopt;
    }
};
