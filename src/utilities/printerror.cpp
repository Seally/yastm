#include "printerror.hpp"

#include "../global.hpp"

void printError(const std::exception& error, const std::size_t depth)
{
    const constexpr std::size_t INDENT_SIZE = 4;
    LOG_ERROR_FMT(
        "{}{}"sv,
        std::string(depth * INDENT_SIZE, ' '),
        error.what());

    try {
        std::rethrow_if_nested(error);
    } catch (const std::exception& e) {
        printError(e, depth + 1);
    } catch (...) {}
}
