#include <memory>

#include <SKSE/SKSE.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <xbyak/xbyak.h>

#include "global.hpp"
#include "version.hpp"

#include "ChargeItemFix.hpp"
#include "EnchantItemFix.hpp"
#include "TrapSoulFix.hpp"

//#include "versiondb.hpp"

//bool DumpOffsets() {
//	VersionDb db;
//
//	if (!db.Load()) {
//		LOG_CRITICAL("Failed to load offset database."sv);
//		return false;
//	}
//
//	const std::string& version{db.GetLoadedVersionString()};
//
//	db.Dump("offsets-" + version + ".txt");
//	LOG_INFO_FMT("Dumped offsets for {}", version);
//
//	return true;
//}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(
    const SKSE::QueryInterface* a_skse,
    SKSE::PluginInfo* a_info)
{
    using namespace std::literals;
    namespace logger = SKSE::log;

    auto path = logger::log_directory();
    if (!path) {
        return false;
    }

    *path /= version::PROJECT;
    *path += ".log"sv;
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        path->string(),
        true);
    auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifndef NDEBUG
    log->set_level(spdlog::level::trace);
    log->flush_on(spdlog::level::trace);
#else
    log->set_level(spdlog::level::info);
    log->flush_on(spdlog::level::info);
#endif

    spdlog::set_default_logger(std::move(log));
    spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);

    LOG_INFO_FMT("{} v{}"sv, version::PROJECT, version::NAME);

    a_info->infoVersion = SKSE::PluginInfo::kVersion;
    a_info->name = version::PROJECT.data();
    a_info->version = version::MAJOR;

    if (a_skse->IsEditor()) {
        LOG_CRITICAL("Loaded in editor, marking as incompatible"sv);
        return false;
    }

    const auto ver = a_skse->RuntimeVersion();
    if (ver < SKSE::RUNTIME_1_5_39) {
        LOG_CRITICAL_FMT("Unsupported runtime version {}"sv, ver.string());
        return false;
    }

    return true;
}

template <typename... FArgs, typename... CArgs>
bool installPatch(
    const std::string_view patchName,
    bool (*patchFunction)(FArgs...),
    CArgs&&... args)
{
    using namespace std::literals;

    try {
        LOG_INFO_FMT("Installing patch \"{}\"..."sv, patchName);
        return patchFunction(std::forward<CArgs>(args)...);
    } catch (const std::exception& e) {
        LOG_ERROR_FMT(
            "Error while installing patch \"{}\": {}"sv,
            patchName,
            e.what());
    }

    return false;
}

extern "C" DLLEXPORT bool SKSEAPI
    SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
    using namespace std::literals;
    LOG_INFO_FMT("Loaded {} v{}"sv, version::PROJECT, version::NAME);

    SKSE::Init(a_skse);

    bool result = installPatch("ChargeItemFix"sv, installChargeItemFix);
    // Use bitwise to avoid short-circuiting.
    result &= installPatch("EnchantItemFix"sv, installEnchantItemFix);

    try {
        result &= installPatch("SoulTrapFix"sv, installTrapSoulFix);
    } catch (const std::exception& error) {
        LOG_ERROR(error.what());
    }

    return result;
}
