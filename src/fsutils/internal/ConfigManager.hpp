#pragma once

#include <map>
#include <shared_mutex>
#include <optional>

#include "Config.hpp"

namespace std {
    namespace filesystem {
        class path;
    }
} // namespace std

class ConfigManager {
public:
    using HandleType = int;

private:
    explicit ConfigManager() {}
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager(ConfigManager&&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    ConfigManager& operator=(ConfigManager&) = delete;

    std::map<HandleType, Config> configs_;
    mutable std::shared_mutex mutex_;

    /**
     * Returns the largest handle that currently exists. Or 0 if there are no
     * handles.
     *
     * Does not lock mutex. All internal users should use this instead of the
     * public version.
     */
    HandleType getLargestHandle_() const noexcept
    {
        const auto largestKey = configs_.rbegin();

        if (largestKey != configs_.rend()) {
            return largestKey->first;
        }

        return 0;
    }

    /**
     * Returns the value of the next handle that will be created.
     *
     * Does not lock mutex. All internal users should use this instead of the
     * public version.
     */
    HandleType getNextHandle_() const noexcept
    {
        return getLargestHandle_() + 1;
    }

public:
    static ConfigManager& getInstance()
    {
        static ConfigManager instance;
        return instance;
    }

    HandleType openConfig(const std::filesystem::path& path);
    HandleType createConfig();

    void closeConfig(HandleType handle);
    bool saveConfig(HandleType handle, const std::filesystem::path& path) const;
    void closeAllConfigs();

    std::size_t size() const noexcept { return configs_.size(); }

    /**
     * Returns the largest handle that currently exists. Or 0 if there are no
     * handles.
     */
    HandleType getLargestHandle() const
    {
        std::shared_lock lock(mutex_);
        return getLargestHandle_();
    }

    /**
     * Returns the value of the next handle that will be created.
     */
    HandleType getNextHandle() const
    {
        std::shared_lock lock(mutex_);
        return getNextHandle_();
    }

    std::optional<std::reference_wrapper<Config>> getConfig(HandleType handle);
};
