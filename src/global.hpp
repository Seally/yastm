#pragma once

#define DLLEXPORT __declspec(dllexport)

// These macros allow potentially zero-overhead log calls (compiled out)
// depending on the project build type.
//
// We set these up here instead of using Spdlog's macros directly so that we can
// still depend on SKSE::log.
#define LOG_CALL(level, ...) SKSE::log::level(__VA_ARGS__)

#ifdef NDEBUG
#    define LOG_TRACE(...)
#    define LOG_DEBUG(...)
#else
#    define LOG_TRACE(...) LOG_CALL(trace, __VA_ARGS__)
#    define LOG_DEBUG(...) LOG_CALL(debug, __VA_ARGS__)
#endif

#define LOG_INFO(...) LOG_CALL(info, __VA_ARGS__)
#define LOG_WARN(...) LOG_CALL(warn, __VA_ARGS__)
#define LOG_ERROR(...) LOG_CALL(error, __VA_ARGS__)
#define LOG_CRITICAL(...) LOG_CALL(critical, __VA_ARGS__)

#define LOG_TRACE_FMT(str, ...) LOG_TRACE(FMT_STRING(str), __VA_ARGS__)
#define LOG_DEBUG_FMT(str, ...) LOG_DEBUG(FMT_STRING(str), __VA_ARGS__)
#define LOG_INFO_FMT(str, ...) LOG_INFO(FMT_STRING(str), __VA_ARGS__)
#define LOG_WARN_FMT(str, ...) LOG_WARN(FMT_STRING(str), __VA_ARGS__)
#define LOG_ERROR_FMT(str, ...) LOG_ERROR(FMT_STRING(str), __VA_ARGS__)
#define LOG_CRITICAL_FMT(str, ...) LOG_CRITICAL(FMT_STRING(str), __VA_ARGS__)

#define VERSION_CODE(primary, secondary, sub)                 \
    (((primary & 0xFFF) << 20) | ((secondary & 0xFFF) << 8) | \
     ((sub & 0xFF) << 0))
#define VERSION_CODE_PRIMARY(in) ((in >> 20) & 0xFFF)
#define VERSION_CODE_SECONDARY(in) ((in >> 8) & 0xFFF)
#define VERSION_CODE_SUB(in) ((in >> 0) & 0xFF)

#if defined(SKYRIM_VERSION_SE)
#    define AE_ONLY(output)
#    define SE_ONLY(output) output
#    define VR_ONLY(output)
#    define VERSION_SPECIFIC_3(se, ae, vr) se
#    define VERSION_SPECIFIC_2(sevr, ae) sevr
/**
 * @brief Like VERSION_SPECIFIC_2, but there's no output for VR. Use when VR is
 * not supported.
 */
#    define VERSION_SPECIFIC_SE(se, ae) se
#elif defined(SKYRIM_VERSION_AE)
#    define AE_ONLY(output) output
#    define SE_ONLY(output)
#    define VR_ONLY(output)
#    define VERSION_SPECIFIC_3(se, ae, vr) ae
#    define VERSION_SPECIFIC_2(sevr, ae) ae
/**
 * @brief Like VERSION_SPECIFIC_2, but there's no output for VR. Use when VR is
 * not supported.
 */
#    define VERSION_SPECIFIC_SE(se, ae) ae
#elif defined(SKYRIM_VERSION_AE2)
#    define AE_ONLY(output) output
#    define SE_ONLY(output)
#    define VR_ONLY(output)
#    define VERSION_SPECIFIC_3(se, ae, vr) ae
#    define VERSION_SPECIFIC_2(sevr, ae) ae
/**
 * @brief Like VERSION_SPECIFIC_2, but there's no output for VR. Use when VR is
 * not supported.
 */
#    define VERSION_SPECIFIC_SE(se, ae) ae
#elif defined(SKYRIM_VERSION_VR)
#    define AE_ONLY(output)
#    define SE_ONLY(output)
#    define VR_ONLY(output) output
#    define VERSION_SPECIFIC_3(se, ae, vr) vr
#    define VERSION_SPECIFIC_2(sevr, ae) sevr
/**
 * @brief Like VERSION_SPECIFIC_2, but there's no output for VR. Use when VR is
 * not supported.
 */
#    define VERSION_SPECIFIC_SE(se, ae)
#else
#    error "SKYRIM_VERSION_<version> is not defined or invalid."
#endif
