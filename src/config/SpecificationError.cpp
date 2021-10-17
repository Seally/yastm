#include "SpecificationError.hpp"

SpecificationError::SpecificationError(const std::string& message)
    : std::runtime_error{message}
{}

SpecificationError::SpecificationError(const char* message)
    : std::runtime_error{message}
{}

InvalidSoulSpecificationError::InvalidSoulSpecificationError(
    const SoulSize givenCapacity,
    const SoulSize givenContainedSoul,
    const std::string& message)
    : SpecificationError{message.empty() ? fmt::format(FMT_STRING("Attempted to look up invalid soul capacity {} and contained soul size {}"sv), givenCapacity, givenContainedSoul) : message}
    , givenCapacity{givenCapacity}
    , givenContainedSoul{givenContainedSoul}
{}

InvalidWhiteSoulSpecificationError::InvalidWhiteSoulSpecificationError(
    const SoulSize givenCapacity,
    const SoulSize givenContainedSoul)
    : InvalidSoulSpecificationError{
          givenCapacity,
          givenContainedSoul,
          fmt::format(
              FMT_STRING(
                  "Attempted to look up invalid white soul capacity {} and contained soul size {}"sv),
              givenCapacity,
              givenContainedSoul)}
{}

InvalidBlackSoulSpecificationError::InvalidBlackSoulSpecificationError(
    const SoulSize givenCapacity,
    const SoulSize givenContainedSoul)
    : InvalidSoulSpecificationError{
          givenCapacity,
          givenContainedSoul,
          fmt::format(
              FMT_STRING(
                  "Attempted to look up invalid black soul capacity {} and contained soul size {}"sv),
              givenCapacity,
              givenContainedSoul)}
{}
