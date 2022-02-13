#pragma once

#include <optional>
#include <string_view>
#include <utility>

#include <cassert>
#include <cstdint>

#include <RE/B/BSCoreTypes.h>
#include <RE/T/TESDataHandler.h>

#include "../global.hpp"

inline std::optional<RE::FormID> toResolvedFormId(
    const RE::FormID localFormId,
    std::string_view modName,
    RE::TESDataHandler* const dataHandler)
{
    assert(dataHandler != nullptr);
    const auto file = dataHandler->LookupModByName(modName);
    if (file == nullptr || file->compileIndex == 0xFF) {
        return std::nullopt;
    }

    RE::FormID resolvedFormId = file->compileIndex << (3 * 8);
    resolvedFormId += file->smallFileCompileIndex << ((1 * 8) + 4);
    resolvedFormId += localFormId;

    return resolvedFormId;
}

/**
 * @brief Returns a pair of mod indices for the given mod name. The indices are
 * returned in the form { fileIndex, smallFileIndex }.
 *
 * If fileIndex == 0xFE, then the user of this function should refer to the
 * smallFileIndex for the actual index.
 */
inline std::optional<std::pair<std::uint8_t, std::uint16_t>>
    getModIndex(std::string_view modName, RE::TESDataHandler* const dataHandler)
{
    auto file = dataHandler->LookupModByName(modName);
    if (file == nullptr || file->compileIndex == 0xFF) {
        return std::nullopt;
    }

    return std::make_pair(file->compileIndex, file->smallFileCompileIndex);
}

inline bool isStandardFormId(const RE::FormID formId)
{
    return formId < 0xFE00'0000;
}

inline bool isTemporaryFormId(const RE::FormID formId)
{
    return formId >= 0xFF00'0000;
}

inline bool isLightFormId(const RE::FormID formId)
{
    return !isStandardFormId(formId) && !isTemporaryFormId(formId);
}

inline std::uint8_t getFileIndex(const RE::FormID formId)
{
    return static_cast<std::uint8_t>((formId & 0xFF00'0000) >> (6 * 4));
}

inline std::uint16_t getSmallFileIndex(const RE::FormID formId)
{
    return static_cast<std::uint16_t>((formId & 0x00FF'F000) >> (3 * 4));
}

inline RE::FormID getLocalFormId(const RE::FormID formId)
{
    if (getFileIndex(formId) == 0xFE) {
        return formId & 0x0000'0FFF;
    }

    return formId & 0x00FF'FFFF;
}

inline const RE::TESFile*
    getModFile(const RE::FormID formId, RE::TESDataHandler* const dataHandler)
{
    assert(dataHandler != nullptr);

    // Temporary references have no mod name.
    if (isTemporaryFormId(formId)) {
        return nullptr;
    }

    // Light plugins.
    if (isLightFormId(formId)) {
        return dataHandler->LookupLoadedLightModByIndex(
            getSmallFileIndex(formId));
    }

    // Normal plugins.
    return dataHandler->LookupLoadedModByIndex(getFileIndex(formId));
}

inline std::optional<std::string_view>
    getModName(const RE::FormID formId, RE::TESDataHandler* const dataHandler)
{
    const auto file = getModFile(formId, dataHandler);

    if (file == nullptr) {
        return std::nullopt;
    }

    return file->GetFilename();
}
