#pragma once

#include <shared_mutex>

#include <toml++/toml.h>

class Config {
    toml::table _data;
    mutable std::shared_mutex _mutex;

public:
    Config() {}
    Config(std::string_view path);

    template <typename T>
    T get(std::string_view key, const T& defaultValue) const
    {
        std::shared_lock lock(_mutex);

        return _data[key].value_or(defaultValue);
    }

    template <typename T>
    void set(std::string_view key, const T value)
    {
        std::unique_lock lock(_mutex);

        _data.insert(key, value);
    }

    bool writeToDisk(const std::filesystem::path& filePath) const;
};
