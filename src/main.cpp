#include <memory>

#include <SKSE/SKSE.h>
#include <xbyak/xbyak.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "global.hpp"
#include "version.hpp"

#include "ChargeItemFix.hpp"
#include "EnchantItemFix.hpp"
#include "SoulTrapFix.hpp"

#include "config/SoulGemConfig.hpp"
//#include "versiondb.hpp"


//bool DumpOffsets() {
//	namespace logger = SKSE::log;
//
//	VersionDb db;
//
//	if (!db.Load()) {
//		logger::critical("Failed to load offset database."sv);
//		return false;
//	}
//
//	const std::string& version{db.GetLoadedVersionString()};
//
//	db.Dump("offsets-" + version + ".txt");
//	logger::info("Dumped offsets for " + version);
//
//	return true;
//}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
    using namespace std::literals;
	namespace logger = SKSE::log;

	auto path = logger::log_directory();
	if (!path) {
		return false;
	}

	*path /= Version::PROJECT;
	*path += ".log"sv;
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifndef NDEBUG
	log->set_level(spdlog::level::trace);
	log->flush_on(spdlog::level::trace);
#else
	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::warn);
#endif

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);

	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Version::PROJECT.data();
	a_info->version = Version::MAJOR;

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_1_5_39) {
		logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
		return false;
	}

	return true;
}

template<typename... FArgs, typename... CArgs>
bool installPatch(const std::string_view patchName, bool(*patchFunction)(FArgs...), CArgs&&... args) {
    namespace logger = SKSE::log;

    try {
        logger::info(FMT_STRING("Installing patch \"{}\"..."), patchName);
        return patchFunction(std::forward<CArgs>(args)...);
    } catch (const std::exception& e) {
        logger::error(FMT_STRING("Error while installing patch \"{}\": {}"), patchName, e.what());
    }

    return false;
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse) {
	namespace logger = SKSE::log;
	logger::info("Loaded.");

	SKSE::Init(a_skse);

	bool result = installPatch("ChargeItemFix", yastm::installChargeItemFix);
	// Use bitwise to avoid short-circuiting.
	result &= installPatch("EnchantItemFix", yastm::installEnchantItemFix);

    try {
        result &= installPatch("SoulTrapFix", yastm::installSoulTrapFix);
    } catch (const std::exception& error) {
        logger::error(error.what());
    }
		
	return result;
}
