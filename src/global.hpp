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
