#pragma once

enum class DLLDependencyKey { ScaleformTranslationPlusPlus };

/**
 * @brief Calls fn(dependencyKey, dependencyName, issueIfMissing) for each available configuration
 * key.
 */
inline void forEachDLLDependencyKey(
    const std::function<void(DLLDependencyKey, const char*, const char*)>& fn)
{
    fn(DLLDependencyKey::ScaleformTranslationPlusPlus,
       "ScaleformTranslationPP",
       "Scaleform Translation Plus Plus not active. Some strings may not be "
       "translated correctly and will use the English fallback.");
}
