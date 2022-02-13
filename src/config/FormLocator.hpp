#pragma once

#include <string>
#include <variant>

#include "FormId.hpp"

using FormLocator = std::variant<FormId, std::string>;
