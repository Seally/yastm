#pragma once

#include <cstdint>

#include "global.hpp"

namespace re {
    namespace fix {
        namespace chargeitem {
            constexpr std::uint8_t expectedBytes[] = {
                // clang-format off
                // [1.5.97.0]  .text:000000014088EB35 
                // [1.6.318.0] .text:00000001408BE27B (bytes are identical)
                // [1.6.323.0] .text:00000001408BDD50 (bytes are identical)
                0x48, 0x85, 0xc0,             // rax is probably BSExtraDataList**
                                              // test    rax, rax            ; TEST performs an implied AND operation that does not modify the destination but sets CPU flags as if it did.
                                              //                             ; ANDing anything against itself produces itself, so this followed by JZ (Jump If Zero) is equivalent to the code:
                                              //                             ; if (rax) { ...<jump destination>... }
                0x74, 0x05,                   // jz      short loc_14088EB3F [1.5.97.0]
                                              // jz      short loc_1408BDFF5 [1.6.323.0]
                0x48, 0x8b, 0x08,             // mov     rcx, [rax]          ; Dereferences rax and assigns it to rcx.
                0xeb, 0x03,                   // jmp     short loc_14088EB42 [1.5.97.0]
                                              // jmp     short loc_1408BDFF8 [1.6.323.0]
                // [1.5.97.0]  loc_14088EB3F:
                // [1.6.323.0] loc_1408BDFF5:
                0x49, 0x8b, 0xcc,             // mov     rcx, r12            ; r12 has been set to 0 for the scope of the function
                // [1.5.97.0]  loc_14088EB42:
                // [1.6.323.0] loc_1408BDFF8:
                0x33, 0xd2,                   // xor     edx, edx            ; equivalent to mov edx, 0 (sets CPU flags as a side effect, but not important here)
                // clang-format on
            };
        } // end namespace chargeitem

        namespace enchantitem {
            constexpr std::uint8_t expectedBytes[] = {
                // clang-format off
                // [1.5.97.0]  .text:000000014086C862
                // [1.6.318.0] .text:000000014089ABE0 (bytes are identical)
                // [1.6.323.0] .text:000000014089A950 (bytes are identical) 
                0x48, 0x85, 0xc9,             // rcx is probably BSExtraDataList**
                                              // test    rcx, rcx            ; TEST performs an implied AND operation that does not modify the destination but sets CPU flags as if it did.
                                              //                             ; ANDing anything against itself produces itself, so this followed by JZ (Jump If Zero) is equivalent to the code:
                                              //                             ; if (rcx) { ...<jump destination>... }
                0x74, 0x05,                   // jz      short loc_14086C86C [1.5.97.0]
                                              //               loc_14089ABEA [1.6.318.0]
                                              //               loc_14089A95A [1.6.323.0]
                0x48, 0x8b, 0x09,             // mov     rcx, [rcx]          ; Dereferences rcx and assigns it to itself.
                0xeb, 0x03,                   // jmp     short loc_14086C86F [1.5.97.0]
                                              //               loc_14089ABED [1.6.318.0]
                                              //               loc_14089A95D [1.6.323.0]
                // [1.5.97.0]  loc_14086C86C:
                // [1.6.318.0] loc_14089ABEA:
                // [1.6.323.0] loc_14089A95A:
                0x49, 0x8b, 0xcd,             // mov     rcx, r13            ; r13 has been set to 0 for the scope of the function
                // [1.5.97.0]  loc_14086C86F:
                // [1.6.318.0] loc_14089ABED:
                // [1.6.323.0] loc_14089A95D:
                0x33, 0xd2,                   // xor     edx, edx            ; equivalent to mov edx, 0
                // clang-format on
            };
        } // end namespace enchantitem

        namespace trapsoul {
            constexpr std::uint8_t expectedPapyrusSoulTrapBytes[] = {
                // clang-format off
                // [1.5.97.0]  .text:000000014094D850
                // [1.6.323.0] .text:000000014097A190 (bytes are identical)
                0x49, 0x8b, 0xd1,             // mov     rdx, r9              ; arg1 <- victim 
                0x49, 0x8b, 0xc8,             // mov     rcx, r8              ; arg0 <- caster
                // Last line jumps to the Actor::TrapSoul() function, but this
                // needs to be handled separately since the jump offset can
                // change so the bytes can't be hardcoded.
                // clang-format on
            };
        } // end namespace trapsoul
    } // end namespace fix
} // end namespace re
