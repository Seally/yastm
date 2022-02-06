#pragma once

#include <utility>

template <typename Container, typename... Args>
void clearContainer(Container& container, Args... args)
{
    container = Container(std::forward(args)...);
}
