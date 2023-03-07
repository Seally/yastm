#include "trapsoulfix.hpp"

#include <exception>

#include <cassert>
#include <cstdint>
#include <cstring>

#include <xbyak/xbyak.h>

#include <fmt/format.h>

#include <SKSE/SKSE.h>
#include <REL/Relocation.h>
#include <RE/M/Misc.h>

#include "global.hpp"
#include "expectedbytes.hpp"
#include "messages.hpp"
#include "offsets.hpp"
#include "trampoline.hpp"
#include "config/ConfigKey/BoolConfigKey.hpp"
#include "config/YASTMConfig.hpp"
#include "trapsoul/trapsoul.hpp"
#include "utilities/assembly.hpp"
#include "utilities/Timer.hpp"
#include "utilities/printerror.hpp"

using namespace std::literals;

namespace {
    bool trapSoul_(RE::Actor* caster, RE::Actor* const victim)
    {
        // This logs the "enter" and "exit" messages upon construction and
        // destruction, respectively.
        //
        // Also prints the time taken to run the function if profiling is
        // enabled (timer will still run if profiling is disabled, just with no
        // visible output).
        class Profiler : public Timer {
        public:
            explicit Profiler() noexcept
            {
                LOG_TRACE("Entering YASTM trapSoul function");
            }

            virtual ~Profiler()
            {
                const auto elapsedTime = elapsed();

                if (YASTMConfig::getInstance().getGlobalBool(
                        BoolConfigKey::AllowProfiling)) {
                    LOG_INFO_FMT(
                        "Time to trap soul: {:.7f} seconds",
                        elapsedTime);
                    RE::DebugNotification(
                        fmt::format(
                            fmt::runtime(
                                getMessage(MiscMessage::TimeTakenToTrapSoul)),
                            elapsedTime)
                            .c_str());
                }

                LOG_TRACE("Exiting YASTM trapSoul function");
            }
        } profiler;

        caster = getProxyCaster(caster);
        return trapSoul(caster, victim);
    }

    bool isPatchable_()
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

    bool installPatch()
    {
        if (!isPatchable_()) {
            return false;
        }

        auto& trampoline = SKSE::GetTrampoline();
        allocateTrampoline();

        LOG_INFO("[TRAPSOUL] Installing Actor::TrapSoul() hijack jump..."sv);
        // Hijack the original Actor::TrapSoul() call so everything that calls
        // it will use our version instead.
        trampoline.write_branch<6>(re::Actor::TrapSoul.address(), trapSoul_);

        return true;
    }

    /**
     * @brief Lookup game forms and construct the soul gem map.
     */
    void handleMessage_(SKSE::MessagingInterface::Message* const message)
    {
        if (message->type == SKSE::MessagingInterface::kDataLoaded) {
            try {
                const auto dataHandler = RE::TESDataHandler::GetSingleton();
                assert(dataHandler != nullptr);
                YASTMConfig::getInstance().loadConfig(dataHandler);
            } catch (const std::exception& error) {
                // If any unrecoverable errors occur, log them.
                printError(error);
                LOG_ERROR("[TRAPSOUL] Configuration initialization failed.");
            }
        }
    }
} // namespace

bool installTrapSoulFix(const SKSE::LoadInterface* const loadInterface)
{
    auto& config = YASTMConfig::getInstance();

    try {
        config.checkDllDependencies(loadInterface);
    } catch (const std::exception& error) {
        LOG_ERROR("Error while checking DLL dependencies:");
        printError(error, 1);
        return false;
    }

    const auto messaging = SKSE::GetMessagingInterface();
    messaging->RegisterListener(handleMessage_);

    return installPatch();
}
