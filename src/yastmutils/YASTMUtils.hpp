#pragma once

namespace SKSE {
    class PapyrusInterface;
}

enum class TrapSoulPatchType {
    Vanilla,
    YASTM,
};

bool registerYASTMUtils(
    TrapSoulPatchType patchType,
    const SKSE::PapyrusInterface* papyrus);
