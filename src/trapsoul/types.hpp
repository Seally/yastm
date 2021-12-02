#pragma once

#include <deque>
#include <queue>

#include "Victim.hpp"
#include "../config/ConfigKey.hpp"

using VictimsQueue = std::priority_queue<Victim, std::deque<Victim>>;

/**
 * @brief Boolean Config Key
 */
using BC = BoolConfigKey;
/**
 * @brief Enum Config Key
 */
using EC = EnumConfigKey;
