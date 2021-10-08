#include "SpecificationError.hpp"

SpecificationError::SpecificationError(const std::string& message)
    : std::runtime_error{message}
{}

InvalidSoulSpecificationError::InvalidSoulSpecificationError(
    const SoulSize givenCapacity,
    const SoulSize givenContainedSoul)
    : SpecificationError{fmt::format(
          FMT_STRING(
              "Attempted to look up invalid soul capacity {} and contained soul size {}"sv),
          givenCapacity,
          givenContainedSoul)}
    , givenCapacity{givenCapacity}
    , givenContainedSoul{givenContainedSoul}
{}
