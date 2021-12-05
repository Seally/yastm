#include "TrapSoulFix.hpp"

#include <xbyak/xbyak.h>

#include <REL/Relocation.h>
#include <SKSE/SKSE.h>

#include "global.hpp"
#include "expectedbytes.hpp"
#include "offsets.hpp"
#include "trampoline.hpp"
#include "config/YASTMConfig.hpp"
#include "trapsoul/trapsoul.hpp"
#include "utilities/assembly.hpp"
#include "utilities/printerror.hpp"

using namespace std::literals;

void _handleMessage(SKSE::MessagingInterface::Message* const message)
{
    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        YASTMConfig::getInstance().loadGameForms(
            RE::TESDataHandler::GetSingleton());
    }
}

bool _isPapyrus_Actor_TrapSoulPatchable()
{
    using namespace re::fix::trapsoul;

    if (std::memcmp(
            reinterpret_cast<std::uint8_t*>(
                re::papyrus::Actor::TrapSoul.address()),
            expectedPapyrusSoulTrapBytes,
            sizeof(expectedPapyrusSoulTrapBytes)) != 0) {
        LOG_CRITICAL(
            "[TRAPSOUL] Expected bytes for papyrus::Actor::TrapSoul() not found."sv);
        return false;
    }

    // Determine the destination of the tail call jump in
    // papyrus::Actor::TrapSoul() and see if it matches our address for
    // Actor::TrapSoul().
    const auto targetAddress =
        InstructionData<Instruction::JMP, 0xe9>::targetAddress(
            re::papyrus::Actor::TrapSoul.address() + branchJmpOffset);

    if (targetAddress != re::Actor::TrapSoul.address()) {
        LOG_CRITICAL(
            "[TRAPSOUL] Unrecognized call to Actor::TrapSoul() in papyrus::Actor::TrapSoul()."sv);
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

    if (!_isPapyrus_Actor_TrapSoulPatchable()) {
        return false;
    }

    auto& trampoline = SKSE::GetTrampoline();
    allocateTrampoline();

    LOG_INFO("[TRAPSOUL] Installing Actor::TrapSoul() hijack jump..."sv);
    // Hijack the original Actor::TrapSoul() call so everything that calls it
    // will use our version instead.
    trampoline.write_branch<5>(re::Actor::TrapSoul.address(), trapSoul);

    return true;
}
