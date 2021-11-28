#include "Config.hpp"

#include <fstream>

// Note to Future Me: Do not handle exceptions here. Let them propagate to the
//                    actual Papyrus call so that we have access to the
//                    Papyrus VM context for logging.

Config::Config(std::string_view path)
    : _data(toml::parse_file(path))
{}

bool Config::writeToDisk(const std::filesystem::path& filePath) const
{
    std::shared_lock lock(_mutex);

    std::ofstream configFile(filePath);
    configFile << _data;

    return true;
}
