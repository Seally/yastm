#pragma once

#include <exception>
#include <string>

class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string& message)
        : std::runtime_error(message)
    {}

    explicit ParseError(const char* message)
        : std::runtime_error(message)
    {}
};
