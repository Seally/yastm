#include "TrapSoulFix.hpp"

#include <future>

#include <cassert>
#include <cstdint>
#include <cstring>

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

namespace {
    class _Patcher {
        std::uint8_t _originalCode[6];
        bool _isPatchInstalled = false;

    public:
        explicit _Patcher() = default;
        _Patcher(const _Patcher&) = delete;
        _Patcher(_Patcher&&) = default;
        _Patcher& operator=(const _Patcher&) = delete;
        _Patcher& operator=(_Patcher&&) = default;

        std::uintptr_t patchAddress() const
        {
            return re::Actor::TrapSoul.address();
        }

        bool isPatchInstalled() const { return _isPatchInstalled; }

        bool isPatchable() const;
        bool installPatch();
        void uninstallPatch();
    };

    bool _Patcher::isPatchable() const
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
                patchAddress() + branchJmpOffset);

        if (targetAddress != re::Actor::TrapSoul.address()) {
            LOG_CRITICAL(
                "[TRAPSOUL] Unrecognized call to Actor::TrapSoul() in papyrus::Actor::TrapSoul()."sv);
            return false;
        }

        return true;
    }

    bool _Patcher::installPatch()
    {
        if (!isPatchable()) {
            return false;
        }

        std::memcpy(
            _originalCode,
            reinterpret_cast<std::uint8_t*>(patchAddress()),
            sizeof(_originalCode));

        auto& trampoline = SKSE::GetTrampoline();
        allocateTrampoline();

        LOG_INFO("[TRAPSOUL] Installing Actor::TrapSoul() hijack jump..."sv);
        // Hijack the original Actor::TrapSoul() call so everything that calls
        // it will use our version instead.
        trampoline.write_branch<6>(patchAddress(), trapSoul);
        _isPatchInstalled = true;

        return true;
    }

    void _Patcher::uninstallPatch()
    {
        if (!_isPatchInstalled) {
            return;
        }

        // Game seems to crash if we use std::memcpy().
        REL::safe_write(patchAddress(), _originalCode, sizeof(_originalCode));
        _isPatchInstalled = false;
    }

    _Patcher _patcher;
    // Used to ensure loadConfigFiles() finishes before starting
    // loadGameForms().
    std::promise<void> _yastmConfigPromise;

    /**
     * @brief Read and parse configuration
     */
    void _loadConfigs(const SKSE::LoadInterface* const loadInterface)
    {
        try {
            auto& config = YASTMConfig::getInstance();
            config.checkDllDependencies(loadInterface);
            config.loadConfigFiles();

            // Signal that we're ready to go to the next stage.
            _yastmConfigPromise.set_value();
        } catch (...) {
            _yastmConfigPromise.set_exception(std::current_exception());
        }
    }

    /**
     * @brief Lookup game forms and construct the soul gem map.
     */
    void _handleMessage(SKSE::MessagingInterface::Message* const message)
    {
        if (message->type == SKSE::MessagingInterface::kDataLoaded) {
            try {
                auto future = _yastmConfigPromise.get_future();

                // Prevent this from being called multiple times.
                if (!future.valid()) {
                    return;
                }

                // Wait for loadConfigFiles() to finish (if it hasn't already).
                future.get();

                const auto dataHandler = RE::TESDataHandler::GetSingleton();
                assert(dataHandler != nullptr);
                YASTMConfig::getInstance().loadGameForms(dataHandler);
            } catch (const std::exception& error) {
                // If any unrecoverable errors occur, log them then revert the
                // patch.
                printError(error);
                LOG_ERROR(
                    "[TRAPSOUL] Configuration initialization failed. "
                    "Uninstalling TrapSoul patch...");
                _patcher.uninstallPatch();
                YASTMConfig::getInstance().clear();
            }
        }
    }
} // namespace

bool installTrapSoulFix(const SKSE::LoadInterface* const loadInterface)
{
    const auto messaging = SKSE::GetMessagingInterface();
    messaging->RegisterListener(_handleMessage);

    _loadConfigs(loadInterface);

    return _patcher.installPatch();
}
