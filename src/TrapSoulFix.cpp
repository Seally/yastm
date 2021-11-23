#include "TrapSoulFix.hpp"

#include <xbyak/xbyak.h>

#include <REL/Relocation.h>
#include <SKSE/SKSE.h>

#include "global.hpp"
#include "config/YASTMConfig.hpp"
#include "trapsoul/trapsoul.hpp"
#include "utilities/printerror.hpp"

#include "expectedbytes.hpp"
#include "offsets.hpp"

using namespace std::literals;

void _handleMessage(SKSE::MessagingInterface::Message* const message)
{
    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        YASTMConfig::getInstance().loadGameForms(
            RE::TESDataHandler::GetSingleton());
    }
}

/**
 * Check if memory has the expected bytes for patching.
 */
bool _isTrapSoulPatchable()
{
    using namespace re::fix::trapsoul;

    if (std::memcmp(
            reinterpret_cast<std::uint8_t*>(
                static_cast<std::uintptr_t>(TrapSoul1.address() + beginOffset)),
            expectedEntryBytes,
            sizeof expectedEntryBytes) != 0) {
        LOG_CRITICAL(
            "[TRAPSOUL] Expected bytes for soul trap handling at call offset not found."sv);
        return false;
    }

    if (std::memcmp(
            reinterpret_cast<std::uint8_t*>(
                static_cast<std::uintptr_t>(TrapSoul1.address() + continueOffset)),
            expectedExitBytes,
            sizeof expectedExitBytes) != 0) {
        LOG_CRITICAL(
            "[TRAPSOUL] Expected bytes for soul trap handling at return offset not found."sv);
        return false;
    }

    return true;
}

bool installTrapSoulFix(const SKSE::LoadInterface* const loadInterface)
{
    using namespace re::fix::trapsoul;

    try {
        auto& config = YASTMConfig::getInstance();
        config.checkDllDependencies(loadInterface);
        config.readConfigs();
    } catch (const std::exception& error) {
        printError(error);
        LOG_ERROR("Not installing trapSoul patch.");

        return false;
    }

    auto messaging = SKSE::GetMessagingInterface();
    messaging->RegisterListener(_handleMessage);

    if (!_isTrapSoulPatchable()) {
        return false;
    }

    // This simply sets up the registers so they will be passed to our
    // replacement function correctly, and jumps back to our original function's
    // ending address.
    struct TrapSoulCall : Xbyak::CodeGenerator {
        explicit TrapSoulCall()
        {
            Xbyak::Label trapSoulLabel;
            Xbyak::Label returnLabel;

            // Set up the arguments and call our function.
            mov(rdx, r9); // victim
            mov(rcx, r8); // caster

            call(ptr[rip + trapSoulLabel]);

            // Jump to original function end.
            jmp(ptr[rip + returnLabel]);

            L(trapSoulLabel);
            dq(reinterpret_cast<std::uint64_t>(trapSoul));

            L(returnLabel);
            dq(TrapSoul1.address() + continueOffset);
        }
    };

    TrapSoulCall patch;
    patch.ready();

    LOG_INFO_FMT("[TRAPSOUL] Patch size: {}"sv, patch.getSize());

    auto& trampoline = SKSE::GetTrampoline();
    trampoline.write_branch<5>(
        TrapSoul1.address() + beginOffset,
        trampoline.allocate(patch));

    return true;
}
