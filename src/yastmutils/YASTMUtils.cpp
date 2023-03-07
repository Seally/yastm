#include "YASTMUtils.hpp"

#include <functional>
#include <sstream>

#include <RE/M/Misc.h>
#include <RE/V/VirtualMachine.h>

#include "../global.hpp"
#include "../messages.hpp"
#include "../config/YASTMConfig.hpp"
#include "../trapsoul/trapsoul.hpp"
#include "../utilities/native.hpp"
#include "../utilities/PapyrusFunctionRegistry.hpp"
#include "../utilities/printerror.hpp"
#include "../utilities/Timer.hpp"

using namespace std::literals;
using RE::BSScript::Internal::VirtualMachine;

namespace {
    RE::Actor* TrapSoulAndGetCaster(
        [[maybe_unused]] VirtualMachine* const vm,
        [[maybe_unused]] RE::VMStackID stackId,
        RE::StaticFunctionTag*,
        RE::Actor* caster,
        RE::Actor* const victim)
    {
        // This logs the "enter" and "exit" messages upon construction and
        // destruction, respectively.
        //
        // Also prints the time taken to run the function if profiling is
        // enabled (timer will still run if profiling is disabled, just with no
        // visible output).
        class Profiler : public Timer {
        public:
            explicit Profiler()
            {
                LOG_TRACE("Entering YASTM trapSoulAndGetCaster function");
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

                LOG_TRACE("Exiting YASTM trapSoulAndGetCaster function");
            }
        } profiler;

        caster = getProxyCaster(caster);
        return trapSoul(caster, victim) ? caster : nullptr;
    }

    bool registerPapyrusFunctions_(VirtualMachine* const vm)
    {
        if (vm == nullptr) {
            LOG_ERROR("Couldn't get VM State"sv);
            return false;
        }

        PapyrusFunctionRegistry registry("YASTMUtils", vm);

        registry.registerFunction("TrapSoulAndGetCaster", TrapSoulAndGetCaster);

        return true;
    }
} // namespace

bool registerYASTMUtils(const SKSE::PapyrusInterface* const papyrus)
{
    return papyrus->Register(registerPapyrusFunctions_);
}
