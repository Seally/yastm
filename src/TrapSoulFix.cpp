#include "TrapSoulFix.hpp"

#include <xbyak/xbyak.h>

#include <REL/Relocation.h>
#include <SKSE/SKSE.h>

#include "global.hpp"
#include "config/YASTMConfig.hpp"
#include "trapsoul/trapsoul.hpp"
#include "utilities/assembly.hpp"
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
bool _isActor_TrapSoulPatchable()
{
    using namespace re::fix::trapsoul;

    if (std::memcmp(
            reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(
                re::Actor::TrapSoul.address() + sigOffset0)),
            expectedSig0Bytes,
            sizeof expectedSig0Bytes) != 0) {
        LOG_CRITICAL(
            "[TRAPSOUL] Expected bytes for soul trap handling at call offset not found."sv);
        return false;
    }

    if (std::memcmp(
            reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(
                re::Actor::TrapSoul.address() + sigOffset1)),
            expectedSig1Bytes,
            sizeof expectedSig1Bytes) != 0) {
        LOG_CRITICAL(
            "[TRAPSOUL] Expected bytes for soul trap handling at return offset not found."sv);
        return false;
    }

    return true;
}

bool _isPapyrus_Actor_TrapSoulPatchable()
{
    using namespace re::fix::trapsoul;

    // Determine the destination of the tail call jump in
    // papyrus::Actor::TrapSoul() and see if it matches our address for
    // Actor::TrapSoul().
    const auto targetAddress =
        InstructionData<Instruction::JMP, 0xe9>::targetAddress(
            re::papyrus::Actor::TrapSoul.address() + branchJmpOffset);

    return targetAddress == re::Actor::TrapSoul.address();
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

    if (!_isPapyrus_Actor_TrapSoulPatchable() ||
        !_isActor_TrapSoulPatchable()) {
        return false;
    }

    auto& trampoline = SKSE::GetTrampoline();

    LOG_INFO("[TRAPSOUL] Installing Papyrus tail call patch..."sv);

    // TODO: Dubious benefit?
    // Replace the function called by Papyrus (saves us a jump).
    trampoline.write_branch<5>(
        re::papyrus::Actor::TrapSoul.address() + branchJmpOffset,
        trapSoul);

    LOG_INFO("[TRAPSOUL] Installing Actor::TrapSoul hijack jump..."sv);
    // Hijack the original Actor::TrapSoul() call with ours so everything
    // else we haven't patched will call it.
    trampoline.write_branch<5>(re::Actor::TrapSoul.address(), trapSoul);

    return true;
}
