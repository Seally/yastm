#include "printerror.hpp"

#include <sstream>

#include "../global.hpp"

namespace {
    constexpr std::size_t INDENT_SIZE_ = 4;
}

void printError(const std::exception& error, const std::size_t depth)
{
    LOG_ERROR_FMT(
        "{}{}"sv,
        std::string(depth * INDENT_SIZE_, ' '),
        error.what());

    try {
        std::rethrow_if_nested(error);
    } catch (const std::exception& e) {
        printError(e, depth + 1);
    } catch (...) {}
}

void printErrorToStream(
    const std::exception& error,
    std::stringstream& stream,
    const std::size_t depth)
{
    stream << std::string(depth * INDENT_SIZE_, ' ') << error.what();

    try {
        std::rethrow_if_nested(error);
    } catch (const std::exception& e) {
        stream << '\n';
        printErrorToStream(e, stream, depth + 1);
    } catch (...) {}
}
