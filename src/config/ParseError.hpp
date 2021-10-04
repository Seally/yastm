#ifndef CONFIG_PARSEERROR_HPP
#define CONFIG_PARSEERROR_HPP

#include <exception>

class ParseError : public std::runtime_error {
    explicit ParseError(std::string_view message)
        : std::runtime_error{message}
    {}
};

#endif // CONFIG_PARSEERROR_HPP
