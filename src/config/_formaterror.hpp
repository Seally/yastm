#ifndef FORMATERROR_H
#define FORMATERROR_H

#include <format>
#include <string>

namespace formatError {
    inline std::string missingOrInvalid(std::string_view objectName, std::size_t index, std::string_view correctType) {
        using namespace std::literals;

        return std::format("{} array required value at index {} is missing or not convertible to a {}."sv, objectName, index, correctType);
    }

    inline std::string missingOrInvalid(std::string_view objectName, std::string_view keyName, std::string_view correctType) {
        using namespace std::literals;

        return std::format("{} required key '{}' is missing or not convertible to a {}."sv, objectName, keyName, correctType);
    }
}

#endif // FORMATERROR_H
