#pragma once

#include <exception>

#include "SoulSize.hpp"

class SpecificationError : public std::runtime_error {
public:
    explicit SpecificationError(const std::string& message);
    explicit SpecificationError(const char* message);
};

class InvalidSoulSpecificationError : public SpecificationError {
public:
    const SoulSize givenCapacity;
    const SoulSize givenContainedSoul;

    explicit InvalidSoulSpecificationError(
        const SoulSize givenCapacity,
        const SoulSize givenContainedSoul,
        const std::string& message = std::string());
};

class InvalidWhiteSoulSpecificationError :
    public InvalidSoulSpecificationError {
public:
    explicit InvalidWhiteSoulSpecificationError(
        const SoulSize givenCapacity,
        const SoulSize givenContainedSoul);
};

class InvalidBlackSoulSpecificationError :
    public InvalidSoulSpecificationError {
public:
    explicit InvalidBlackSoulSpecificationError(
        const SoulSize givenCapacity,
        const SoulSize givenContainedSoul);
};
