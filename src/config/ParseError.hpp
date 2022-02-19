#pragma once

#include <exception>

class ParseError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};
