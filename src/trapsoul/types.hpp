#pragma once

#include <deque>
#include <queue>

#include "Victim.hpp"
#include "../config/ConfigKey/BoolConfigKey.hpp"
#include "../config/ConfigKey/EnumConfigKey.hpp"
#include "../config/ConfigKey/IntConfigKey.hpp"

using VictimsQueue = std::priority_queue<Victim, std::deque<Victim>>;

/**
 * @brief Boolean Config Key
 */
using BC = BoolConfigKey;
/**
 * @brief Enum Config Key
 */
using EC = EnumConfigKey;
/**
 * @brief Int Config Key 
 */
using IC = IntConfigKey;
