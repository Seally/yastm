#include "SoulGemMap.hpp"

#include <algorithm>
#include <unordered_map>
#include <vector>

#include <cmath>
#include <cstring>

#include <RE/F/FormTypes.h>
#include <RE/T/TESDataHandler.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESSoulGem.h>
#include <RE/S/SoulLevels.h>

#include <fmt/format.h>

#include "../global.hpp"
#include "FormError.hpp"
#include "SoulGemGroup.hpp"
#include "../SoulSize.hpp"
#include "../SoulValue.hpp"
#include "../utilities/containerutils.hpp"
#include "../utilities/misc.hpp"
#include "../utilities/native.hpp"
#include "../utilities/printerror.hpp"
#include "../formatters/TESSoulGem.hpp"

using namespace std::literals;

namespace {
    /**
     * @brief Returns the capacity number to be used for sorting the soul gem.
     *
     * @details Base sort capacity starts at 1 (petty) to 6 (black). If the soul
     * gem is reusable, +0.5 is added to the base sort capacity (so it will
     * "higher" than non-reusable soul gems of the same capacity).
     */
    double getSortCapacity_(const RE::TESSoulGem* const soulGem)
    {
        double sortCapacity;

        if (canHoldBlackSoul(soulGem)) {
            sortCapacity = 6;
        } else {
            sortCapacity = static_cast<double>(soulGem->GetMaximumCapacity());
        }

        if (soulGem->HasKeyword(getReusableSoulGemKeyword())) {
            sortCapacity += 0.5;
        }

        return sortCapacity;
    };

    /**
     * @brief Compares two soul gems (ascending) and returns if left should be
     * sorted before right.
     *
     * @details This function sorts by capacity (@ref getSortCapacity_), name,
     * and contained soul size, respectively.
     */
    bool compareSoulGems_(
        const RE::TESSoulGem* const left,
        const RE::TESSoulGem* const right)
    {
        const auto leftCapacity = getSortCapacity_(left);
        const auto rightCapacity = getSortCapacity_(right);

        if (leftCapacity != rightCapacity) {
            return leftCapacity < rightCapacity;
        }

        int nameDiff = std::strcmp(left->GetName(), right->GetName());

        if (nameDiff != 0) {
            return nameDiff < 0;
        }

        return left->GetContainedSoul() < right->GetContainedSoul();
    };
} // end namespace

void SoulGemMap::initializeWith(
    RE::TESDataHandler* dataHandler,
    const std::function<void(Transaction&)>& fn)
{
    Transaction t;

    fn(t);

    GroupListMap capacityToGroupListMap;
    BaseFormMap gemToBaseFormMap;

    using MapKey = SoulGemGroup::MemberList::value_type;

    auto addGroupToBaseFormMap = [&](const ConcreteSoulGemGroup& group) {
        const auto baseSoulGem = group.at(SoulSize::None);
        for (const auto& [soulSize, soulGem] : group) {
            gemToBaseFormMap.emplace(soulGem, baseSoulGem);
        }
    };

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
    forEachLoadPriority([&](const LoadPriority priority) {
        for (const auto& group : t.groupsToAdd_) {
            try {
                if (group.get().priority() == priority) {
                    // Add black soul gems to the map.
                    if (group.get().capacity() == SoulGemCapacity::Black) {
                        LOG_INFO_FMT(
                            "- Loading soul gems for {:c}",
                            group.get());

                        const auto& addedGroup =
                            capacityToGroupListMap[SoulGemCapacity::Black]
                                .emplace_back(new ConcreteSoulGemGroup(
                                    group,
                                    dataHandler));

                        blackSoulGemGroupMap.emplace(
                            group.get().emptyMember(),
                            addedGroup.get());

                        addGroupToBaseFormMap(*addedGroup);
                    }
                }
            } catch (const std::exception& error) {
                printError(error);
            }
        }
    });

    LOG_INFO("Loading other soul gem groups");
    forEachLoadPriority([&](const LoadPriority priority) {
        for (const auto& group : t.groupsToAdd_) {
            try {
                if (group.get().priority() == priority) {
                    const auto capacity = group.get().capacity();

                    if (capacity == SoulGemCapacity::Grand) {
                        LOG_INFO_FMT("- Loading soul gems for {}", group.get());

                        // If the empty soul gem form are the same as the empty
                        // form of one of the black soul gem groups, we're
                        // dealing with a dual soul gem group.
                        auto it = blackSoulGemGroupMap.find(
                            group.get().emptyMember());

                        if (it != blackSoulGemGroupMap.end()) {
                            // Group is a dual soul gem group.
                            const auto& addedGroup =
                                capacityToGroupListMap[SoulGemCapacity::Dual]
                                    .emplace_back(new ConcreteSoulGemGroup(
                                        group,
                                        *it->second,
                                        dataHandler));

                            addGroupToBaseFormMap(*addedGroup);
                        } else {
                            // Group is a normal grand soul gem group.
                            const auto& addedGroup =
                                capacityToGroupListMap[SoulGemCapacity::Grand]
                                    .emplace_back(new ConcreteSoulGemGroup(
                                        group,
                                        dataHandler));

                            addGroupToBaseFormMap(*addedGroup);
                        }
                    } else if (capacity != SoulGemCapacity::Black) {
                        LOG_INFO_FMT("- Loading soul gems for {}", group.get());

                        const auto& addedGroup =
                            capacityToGroupListMap[capacity].emplace_back(
                                new ConcreteSoulGemGroup(group, dataHandler));

                        addGroupToBaseFormMap(*addedGroup);
                    }
                }
            } catch (const std::exception& error) {
                printError(error);
            }
        }
    });

    // Assign it if we reach this point so we don't end in a half-initialized
    // state.
    soulGemMap_ = std::move(capacityToGroupListMap);
    baseFormMap_ = std::move(gemToBaseFormMap);
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
                LOG_INFO_FMT("- {:f}", *it);
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

    LOG_INFO("Base form mappings:");

    using BaseFormMapEntryList =
        std::vector<std::pair<RE::TESSoulGem*, RE::TESSoulGem*>>;

    BaseFormMapEntryList baseFormMapEntries(
        baseFormMap_.begin(),
        baseFormMap_.end());

    std::sort(
        baseFormMapEntries.begin(),
        baseFormMapEntries.end(),
        [](const BaseFormMapEntryList::value_type& left,
           const BaseFormMapEntryList::value_type& right) {
            return compareSoulGems_(left.first, right.first);
        });

    for (const auto [soulGem, baseSoulGem] : baseFormMapEntries) {
        LOG_INFO_FMT("{} => {}", *soulGem, *baseSoulGem);
    }
}
