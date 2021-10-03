#ifndef CONFIG_PARSEERROR_H
#define CONFIG_PARSEERROR_H

#include <exception>

class ParseError : public std::runtime_error {
    explicit ParseError(std::string_view message)
        : std::runtime_error{message}
    {}
};

#endif // CONFIG_PARSEERROR_H
