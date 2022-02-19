#pragma once

#include <exception>
#include <string_view>

#include "SoulSize.hpp"

class SpecificationError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class InvalidSoulSpecificationError : public SpecificationError {
public:
    const SoulSize givenCapacity;
    const SoulSize givenContainedSoul;

    explicit InvalidSoulSpecificationError(
        SoulSize givenCapacity,
        SoulSize givenContainedSoul,
        std::string_view message = "");
};

class InvalidWhiteSoulSpecificationError :
    public InvalidSoulSpecificationError {
public:
    explicit InvalidWhiteSoulSpecificationError(
        SoulSize givenCapacity,
        SoulSize givenContainedSoul);
};

class InvalidBlackSoulSpecificationError :
    public InvalidSoulSpecificationError {
public:
    explicit InvalidBlackSoulSpecificationError(
        SoulSize givenCapacity,
        SoulSize givenContainedSoul);
};
