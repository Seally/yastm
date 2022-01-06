#include "ConfigManager.hpp"

#include <filesystem>

// Note to Future Me: Do not handle exceptions here. Let them propagate to the
//                    actual Papyrus call so that we have access to the
//                    Papyrus VM context for logging.

using HandleType = ConfigManager::HandleType;

HandleType ConfigManager::openConfig(const std::filesystem::path& filePath)
{
    std::unique_lock lock(mutex_);

    // Do NOT use the public function otherwise we'll end up in a deadlock.
    const auto handle = getNextHandle_();
    configs_.emplace(handle, filePath.string());
    return handle;
}

HandleType ConfigManager::createConfig()
{
    std::unique_lock lock(mutex_);

    // Do NOT use the public function otherwise we'll end up in a deadlock.
    const auto handle = getNextHandle_();
    configs_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(handle),
        std::tuple<>());

    return handle;
}

void ConfigManager::closeConfig(const HandleType handle)
{
    std::unique_lock lock(mutex_);

    configs_.erase(handle);
}

bool ConfigManager::saveConfig(
    const HandleType handle,
    const std::filesystem::path& filePath) const
{
    std::shared_lock lock(mutex_);

    auto it = configs_.find(handle);

    if (it == configs_.end()) {
        // Handle does not exist.
        return false;
    }

    const auto& config = it->second;
    config.writeToDisk(filePath);
    return true;
}

std::optional<std::reference_wrapper<Config>>
    ConfigManager::getConfig(const HandleType handle)
{
    std::shared_lock lock(mutex_);

    auto it = configs_.find(handle);

    if (it == configs_.end()) {
        return std::nullopt;
    }

    return it->second;
}

void ConfigManager::closeAllConfigs()
{
    std::unique_lock handle(mutex_);
    configs_.clear();
}
