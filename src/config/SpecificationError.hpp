#pragma once

#include <exception>

#include "SoulSize.hpp"

class SpecificationError : public std::runtime_error {
public:
    explicit SpecificationError(const std::string& message);
};

class InvalidSoulSpecificationError : public SpecificationError {
public:
    const SoulSize givenCapacity;
    const SoulSize givenContainedSoul;

    explicit InvalidSoulSpecificationError(
        const SoulSize givenCapacity,
        const SoulSize givenContainedSoul);
};
