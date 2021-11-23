#pragma once

#include <cstdint>

#include "global.hpp"

namespace re {
    namespace fix {
        namespace chargeitem {
            constexpr std::uint8_t expectedBytes[] = {
                // clang-format off
                // .text:000000014088EB35 [1.5.97.0]
                // .text:00000001408BE27B [1.6.318.0] (bytes are identical)

                // [1.5.97.0]  loc_14088EB35:
                // [1.6.318.0] loc_1408BE27B:
                0x48, 0x85, 0xc0,             // rax is probably ExtraDataList
                                              // test    rax, rax            ; TEST performs an implied AND operation that does not modify the destination but sets CPU flags as if it did.
                                              //                             ; ANDing anything against itself produces itself, so this followed by JZ (Jump If Zero) is equivalent to the code:
                                              //                             ; if (rax) { ...<jump destination>... }
                0x74, 0x05,                   // jz      short loc_14055EB3F
                0x48, 0x8b, 0x08,             // mov     rcx, [rax]          ; Dereferences rcx and assigns it to itself.
                0xeb, 0x03,                   // jmp     short loc_14088EB42
                // loc_14088EB3F:
                0x49, 0x8b, 0xcc,             // mov     rcx, r12            ; r12 has been set to 0 for the scope of the function
                // loc_14088EB42:
                0x33, 0xd2,                   // xor     edx, edx            ; equivalent to mov edx, 0
                // clang-format on
            };
        } // end namespace chargeitem

        namespace enchantitem {
            constexpr std::uint8_t expectedBytes[] = {
                // clang-format off
                // [1.5.97.0]  .text:000000014086C862
                // [1.6.318.0] .text:000000014089ABE0 (bytes are identical)

                // [1.5.97.0]  loc_14086C862:
                // [1.6.318.0] loc_14089ABE0:
                0x48, 0x85, 0xc9,             // rcx is probably ExtraDataList
                                              // test    rcx, rcx            ; TEST performs an implied AND operation that does not modify the destination but sets CPU flags as if it did.
                                              //                             ; ANDing anything against itself produces itself, so this followed by JZ (Jump If Zero) is equivalent to the code:
                                              //                             ; if (rcx) { ...<jump destination>... }
                0x74, 0x05,                   // jz      short loc_14086C86C [1.5.97.0]
                                              //               loc_14089ABEA [1.6.318.0]
                0x48, 0x8b, 0x09,             // mov     rcx, [rcx]          ; Dereferences rcx and assigns it to itself.
                0xeb, 0x03,                   // jmp     short loc_14086C86F [1.5.97.0]
                                              //               loc_14089ABED [1.6.318.0]
                // [1.5.97.0]  loc_14086C86C:
                // [1.6.318.0] loc_14089ABEA:
                0x49, 0x8b, 0xcd,             // mov     rcx, r13            ; r13 has been set to 0 for the scope of the function
                // [1.5.97.0]  loc_14086C86F:
                // [1.6.318.0] loc_14089ABED:
                0x33, 0xd2,                   // xor     edx, edx            ; equivalent to mov edx, 0
                // clang-format on
            };
        } // end namespace enchantitem

        namespace trapsoul {
            constexpr std::uint8_t expectedEntryBytes[] = {
                // clang-format off
                // [1.5.97.0]  .text:0000000140634917
                // [1.6.318.0] .text:000000014065A9E7 (bytes are identical)
                0x48, 0x89, 0x58, 0x10,             // mov  [rax+10h], rbx
                0x48, 0x89, 0x68, 0x18,             // mov  [rax+18h], rbp
                0x48, 0x8b, 0xf2,                   // mov  rsi, rdx
                0x4c, 0x8b, 0xf1,                   // mov  r14, rcx
                0x40, 0x32, 0xff,                   // xor  dil, dil
                0x48, 0x8b, 0x01,                   // mov  rax, [rcx]
                0x33, 0xd2,                         // xor  edx, edx
                0xff, 0x90, 0xc8, 0x04, 0x00, 0x00, // call qword ptr [rax+4C8h]
                // clang-format on
            };

            constexpr std::uint8_t expectedExitBytes[] = {
                // clang-format off
                // [1.5.97.0]  .text:0000000140634B56
                // [1.6.318.0] .text:0000000140634B56 (bytes are identical)
                0x4c, 0x8d, 0x5c, 0x24, 0x70, // lea r11, [rsp+98h+var_28]
                0x49, 0x8b, 0x5b, 0x38,       // mov rbx, [r11+38h]
                0x49, 0x8b, 0x6b, 0x40,       // mov rbp, [r11+40h]
                0x49, 0x8b, 0xe3,             // mov rsp, r11
                // clang-format on
            };
        } // end namespace trapsoul
    } // end namespace fix
} // end namespace re
