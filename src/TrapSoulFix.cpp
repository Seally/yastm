#include "TrapSoulFix.hpp"

#include <xbyak/xbyak.h>

#include <REL/Relocation.h>
#include <SKSE/SKSE.h>

#include "global.hpp"
#include "config/YASTMConfig.hpp"
#include "trapsoul/trapsoul.hpp"
#include "utilities/printerror.hpp"

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
bool _isTrapSoulPatchable(
    const std::uintptr_t baseAddress,
    const std::uintptr_t callOffset,
    const std::uintptr_t returnOffset)
{
    const std::uint8_t expectedEntry[] = {
        // clang-format off
        // [1.5.97.0]  .text:0000000140634917
        // [1.6.318.0] .text:000000014065A9E7 (bytes are identical)
        0x48, 0x89, 0x58, 0x10,            // mov  [rax+10h], rbx
        0x48, 0x89, 0x68, 0x18,            // mov  [rax+18h], rbp
        0x48, 0x8b, 0xf2,                  // mov  rsi, rdx
        0x4c, 0x8b, 0xf1,                  // mov  r14, rcx
        0x40, 0x32, 0xff,                  // xor  dil, dil
        0x48, 0x8b, 0x01,                  // mov  rax, [rcx]
        0x33, 0xd2,                        // xor  edx, edx
        0xff, 0x90, 0xc8, 0x04, 0x00, 0x00 // call qword ptr [rax+4C8h]
        // clang-format on
    };

    const std::uint8_t expectedExit[] = {
        // clang-format off
        // [1.5.97.0]  .text:0000000140634B56
        // [1.6.318.0] .text:0000000140634B56 (bytes are identical)
        0x4c, 0x8d, 0x5c, 0x24, 0x70, // lea r11, [rsp+98h+var_28]
        0x49, 0x8b, 0x5b, 0x38,       // mov rbx, [r11+38h]
        0x49, 0x8b, 0x6b, 0x40,       // mov rbp, [r11+40h]
        0x49, 0x8b, 0xe3,             // mov rsp, r11
        // clang-format on
    };

    if (std::memcmp(
            reinterpret_cast<std::uint8_t*>(
                static_cast<std::uintptr_t>(baseAddress + callOffset)),
            expectedEntry,
            sizeof expectedEntry) != 0) {
        LOG_CRITICAL(
            "[TRAPSOUL] Expected bytes for soul trap handling at call offset not found."sv);
        return false;
    }

    if (std::memcmp(
            reinterpret_cast<std::uint8_t*>(
                static_cast<std::uintptr_t>(baseAddress + returnOffset)),
            expectedExit,
            sizeof expectedExit) != 0) {
        LOG_CRITICAL(
            "[TRAPSOUL] Expected bytes for soul trap handling at return offset not found."sv);
        return false;
    }

    return true;
}

bool installTrapSoulFix(const SKSE::LoadInterface* const loadInterface)
{
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

    // [soulTrap1_id]
    //
    // SkyrimSE.exe + 0x634900 [1.5.97.0]  [ADDRLIB:37863]
    // SkyrimSE.exe + 0x65a9d0 [1.6.318.0] [ADDRLIB:38818]
    const REL::ID soulTrap1_id(38818);
    constexpr std::uintptr_t returnOffset = 0x282; // 0x256 [1.5.97.0]
                                                   // 0x282 [1.6.318.0]
    constexpr std::uintptr_t callOffset = 0x17;    // Same in AE

    if (!_isTrapSoulPatchable(
            soulTrap1_id.address(),
            callOffset,
            returnOffset)) {
        return false;
    }

    // This simply sets up the registers so they will be passed to our
    // replacement function correctly, and jumps back to our original function's
    // ending address.
    struct TrapSoulCall : Xbyak::CodeGenerator {
        explicit TrapSoulCall(
            const REL::ID& soulTrap1_id,
            const std::uintptr_t returnOffset)
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
            dq(soulTrap1_id.address() + returnOffset);
        }
    };

    TrapSoulCall patch{soulTrap1_id, returnOffset};
    patch.ready();

    LOG_INFO_FMT("[TRAPSOUL] Patch size: {}"sv, patch.getSize());

    auto& trampoline = SKSE::GetTrampoline();
    trampoline.write_branch<5>(
        soulTrap1_id.address() + callOffset,
        trampoline.allocate(patch));

    return true;
}
