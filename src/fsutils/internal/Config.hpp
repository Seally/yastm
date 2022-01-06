#pragma once

#include <shared_mutex>

#include <toml++/toml.h>

class Config {
    toml::table data_;
    mutable std::shared_mutex mutex_;

public:
    Config() {}
    Config(std::string_view path);

    bool has(std::string_view key) const
    {
        std::shared_lock lock(mutex_);
        return data_.contains(key);
    }

    template <typename T>
    T get(std::string_view key, const T& defaultValue) const
    {
        std::shared_lock lock(mutex_);

        return data_[key].value_or(defaultValue);
    }

    template <typename T>
    void set(std::string_view key, const T value)
    {
        std::unique_lock lock(mutex_);

        data_.insert(key, value);
    }

    bool writeToDisk(const std::filesystem::path& filePath) const;
};
