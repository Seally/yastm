#pragma once

enum class DllDependencyKey { ScaleformTranslationPlusPlus };

/**
 * @brief Calls fn(dependencyKey, dependencyName, issueIfMissing) for each available configuration
 * key.
 */
inline void forEachDllDependencyKey(
    const std::function<void(DllDependencyKey, const char*, const char*)>& fn)
{
    fn(DllDependencyKey::ScaleformTranslationPlusPlus,
       "ScaleformTranslationPP",
       "Scaleform Translation Plus Plus not active. Some strings may not be translated correctly and will use the English fallback.");
}
