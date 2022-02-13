#include "SoulGemMap.hpp"

#include <unordered_map>

#include <RE/F/FormTypes.h>
#include <RE/T/TESDataHandler.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESSoulGem.h>
#include <RE/S/SoulLevels.h>

#include <fmt/format.h>

#include "../global.hpp"
#include "../SoulValue.hpp"
#include "FormError.hpp"
#include "SoulGemGroup.hpp"
#include "../utilities/containerutils.hpp"
#include "../utilities/printerror.hpp"
#include "../utilities/native.hpp"
#include "../formatters/TESSoulGem.hpp"

using namespace std::literals;

void SoulGemMap::initializeWith(
    RE::TESDataHandler* dataHandler,
    const std::function<void(Transaction&)>& fn)
{
    Transaction t;

    fn(t);

    FormMap soulGemGroupsByCapacity;

    using MapKey = SoulGemGroup::MemberList::value_type;

    /**
     * @brief Stores a map of the empty soul gem FormId to its soul gem group. 
    */
    std::unordered_map<
        std::reference_wrapper<const MapKey>,
        const ConcreteSoulGemGroup*,
        std::hash<MapKey>,
        std::equal_to<MapKey>>
        blackSoulGemGroupMap;

    LOG_INFO("Loading black soul gem groups");
    // Black soul gem groups are added first since we need to construct a map to
    // identify dual soul gem groups.
    forEachLoadPriority([&, this](const LoadPriority priority) {
        for (const auto& group : t.groupsToAdd_) {
            try {
                if (group.get().priority() == priority) {
                    LOG_INFO_FMT("- Loading soul gems for {:c}", group.get());

                    // Add black soul gems to the map.
                    if (group.get().capacity() == SoulGemCapacity::Black) {
                        const auto& addedGroup =
                            soulGemGroupsByCapacity[SoulGemCapacity::Black]
                                .emplace_back(new ConcreteSoulGemGroup(
                                    group,
                                    dataHandler));

                        blackSoulGemGroupMap.emplace(
                            group.get().emptyMember(),
                            addedGroup.get());
                    }
                }
            } catch (const std::exception& error) {
                printError(error);
            }
        }
    });

    LOG_INFO("Loading other soul gem groups");
    forEachLoadPriority([&, this](const LoadPriority priority) {
        for (const auto& group : t.groupsToAdd_) {
            try {
                if (group.get().priority() == priority) {
                    const auto capacity = group.get().capacity();

                    if (capacity == SoulGemCapacity::Grand) {
                        LOG_INFO_FMT("- Loading soul gems for {}", group.get());

                        auto it = blackSoulGemGroupMap.find(
                            group.get().emptyMember());

                        if (it != blackSoulGemGroupMap.end()) {
                            // Group is a dual soul gem group.
                            soulGemGroupsByCapacity[SoulGemCapacity::Dual]
                                .emplace_back(new ConcreteSoulGemGroup(
                                    group,
                                    *it->second,
                                    dataHandler));
                        } else {
                            // Group is a normal grand soul gem group.
                            soulGemGroupsByCapacity[SoulGemCapacity::Grand]
                                .emplace_back(new ConcreteSoulGemGroup(
                                    group,
                                    dataHandler));
                        }
                    } else if (capacity != SoulGemCapacity::Black) {
                        LOG_INFO_FMT("- Loading soul gems for {}", group.get());

                        soulGemGroupsByCapacity[capacity].emplace_back(
                            new ConcreteSoulGemGroup(group, dataHandler));
                    }
                }
            } catch (const std::exception& error) {
                printError(error);
            }
        }
    });

    LOG_INFO("Initialize missing capacity vectors");
    // Initialize the missing vectors for capacities if not already constructed.
    forEachSoulGemCapacity([&](const SoulGemCapacity capacity) {
        soulGemGroupsByCapacity.try_emplace(capacity);
    });

    // Assign it if we reach this point so we don't end in a half-initialized
    // state.
    soulGemMap_ = std::move(soulGemGroupsByCapacity);
}

void SoulGemMap::clear() { clearContainer(soulGemMap_); }

void SoulGemMap::printContents() const
{
    const auto printSoulGemsWith = [this](
                                       const SoulGemCapacity capacity,
                                       const SoulSize containedSoulSize) {
        LOG_INFO_FMT(
            "Listing mapped soul gems with capacity={:t} "
            "containedSoulSize={:t}:",
            capacity,
            containedSoulSize);

        const auto [begin, end] = getSoulGemsWith(capacity, containedSoulSize);

        for (auto it = begin; it != end; ++it) {
            if (it.get() == nullptr) {
                LOG_INFO("- null");
            } else {
                LOG_INFO_FMT("- {}", *it);
            }
        }
    };

    try {
        for (SoulGemCapacityValue capacity = SoulGemCapacity::First;
             capacity <= SoulGemCapacity::LastWhite;
             ++capacity) {
            for (SoulSizeValue containedSoulSize = SoulSize::First;
                 containedSoulSize <= capacity;
                 ++containedSoulSize) {
                printSoulGemsWith(capacity, containedSoulSize);
            }
        }

        printSoulGemsWith(SoulGemCapacity::Black, SoulSize::None);
        printSoulGemsWith(SoulGemCapacity::Black, SoulSize::Black);
    } catch (const std::exception& error) {
        printError(error);
    }
}
