#include "ConfigManager.hpp"

#include <filesystem>

// Note to Future Me: Do not handle exceptions here. Let them go to the actual
//                    Papyrus call so we that have access to the Papyrus VM
//                    context for logging.

using HandleType = ConfigManager::HandleType;

HandleType ConfigManager::openConfig(const std::filesystem::path& filePath)
{
    std::unique_lock lock(_mutex);

    // Do NOT use the public function otherwise we'll end up in a deadlock.
    const auto handle = _getNextHandle();
    _configs.emplace(handle, filePath.string());
    return handle;
}

HandleType ConfigManager::createConfig()
{
    std::unique_lock lock(_mutex);

    // Do NOT use the public function otherwise we'll end up in a deadlock.
    const auto handle = _getNextHandle();
    _configs.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(handle),
        std::tuple<>());

    return handle;
}

void ConfigManager::closeConfig(const HandleType handle)
{
    std::unique_lock lock(_mutex);

    _configs.erase(handle);
}

bool ConfigManager::saveConfig(
    const HandleType handle,
    const std::filesystem::path& filePath) const
{
    std::shared_lock lock(_mutex);

    auto it = _configs.find(handle);

    if (it == _configs.end()) {
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
    std::shared_lock lock(_mutex);

    auto it = _configs.find(handle);

    if (it == _configs.end()) {
        return std::nullopt;
    }

    return it->second;
}

void ConfigManager::closeAllConfigs()
{
    std::unique_lock handle(_mutex);
    _configs.clear();
}
