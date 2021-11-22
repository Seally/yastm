cmake_minimum_required(VERSION 3.21)

# ---- Options ----

option(COPY_BUILD "Copy the build output to target directory." OFF)
set(SKYRIM64_DATA_PATH "" CACHE PATH "Path to the Skyrim SE Data folder. Hint: You can set this to the mod folder when using MO2.")

# ---- Cache build vars ----

macro(set_from_environment VARIABLE)
    if(NOT DEFINED ${VARIABLE} AND DEFINED ENV{${VARIABLE}})
        set(${VARIABLE} $ENV{${VARIABLE}})
    endif()
endmacro()

set_from_environment(Skyrim64Path)
set_from_environment(VCPKG_ROOT)

if(DEFINED VCPKG_ROOT)
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "")
    set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
else()
    message(
        WARNING
        "Variable VCPKG_ROOT is not set. Continuing without vcpkg."
    )
endif()

set(Boost_USE_STATIC_RUNTIME OFF CACHE BOOL "")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "")

# ---- Project ----

project(
    YASTM
    VERSION 1.7.3
    LANGUAGES CXX
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/version.hpp
    @ONLY
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    @ONLY
)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(
        FATAL_ERROR
            "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
    )
endif()

# ---- Globals ----

if(MSVC)
    add_compile_definitions(
        _UNICODE
    )

    if(NOT ${CMAKE_GENERATOR} STREQUAL "Ninja")
        add_compile_options(
            /MP # Build with Multiple Processes
        )
    endif()
endif()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

set(Boost_USE_STATIC_LIBS ON)


# ---- Dependencies ----

add_subdirectory("extern/CommonLibSSE" CommonLibSSE)

find_package(spdlog REQUIRED CONFIG)
find_package(tomlplusplus REQUIRED)

# ---- Add source files ----

set(SOURCES
    src/global.hpp
    src/main.cpp
    src/PCH.hpp
    src/ChargeItemFix.hpp
    src/ChargeItemFix.cpp
    src/EnchantItemFix.hpp
    src/EnchantItemFix.cpp
    src/messages.hpp
    src/SoulValue.hpp
    src/TrapSoulFix.hpp
    src/TrapSoulFix.cpp
    src/versiondb.hpp
    src/Victim.hpp
    src/Victim.cpp
    src/config/ConcreteSoulGemGroup.hpp
    src/config/ConcreteSoulGemGroup.cpp
    src/config/ConfigKey.hpp
    src/config/DLLDependencyKey.hpp
    src/config/Form.hpp
    src/config/FormError.hpp
    src/config/FormError.cpp
    src/config/FormId.hpp
    src/config/FormId.cpp
    src/config/GlobalVariable.hpp
    src/config/LoadPriority.hpp
    src/config/ParseError.hpp
    src/config/ParseError.cpp
    src/config/SoulGemGroup.hpp
    src/config/SoulGemGroup.cpp
    src/config/SoulGemMap.hpp
    src/config/SoulGemMap.cpp
    src/config/SoulSize.hpp
    src/config/SpecificationError.hpp
    src/config/SpecificationError.cpp
    src/config/YASTMConfig.hpp
    src/config/YASTMConfig.cpp
    src/formatters/TESSoulGem.hpp
    src/utilities/algorithms.hpp
    src/utilities/FormType.hpp
    src/utilities/printerror.hpp
    src/utilities/printerror.cpp
    src/utilities/soultraputilities.hpp
    src/utilities/soultraputilities.cpp
    src/utilities/stringutils.hpp
    src/utilities/TESObjectREFR.hpp
    src/utilities/TESObjectREFR.cpp
    src/utilities/Timer.hpp
    src/RE/S/SoulsTrapped.hpp
    src/RE/S/SoulsTrapped.cpp
)

source_group(
    TREE
        ${CMAKE_CURRENT_SOURCE_DIR}
    FILES
        ${SOURCES}
)

source_group(
    TREE
        ${CMAKE_CURRENT_BINARY_DIR}
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/include/version.hpp
)

# ---- Create DLL ----

add_library(
    ${PROJECT_NAME}
    SHARED
    ${SOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}/include/version.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    .clang-format
    .editorconfig
)

target_compile_features(
    ${PROJECT_NAME}
    PRIVATE
        cxx_std_20
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        CommonLibSSE::CommonLibSSE
        spdlog::spdlog
        fmt::fmt
        tomlplusplus::tomlplusplus
)

if(MSVC)
    target_compile_options(
        ${PROJECT_NAME}
        PRIVATE
            /sdl                    # Enable Additional Security Checks
            /utf-8                  # Set Source and Executable character sets to UTF-8
            /Zi                     # Debug Information Format

            /permissive-            # Standards conformance

            /Zc:alignedNew          # C++17 over-aligned allocation
            /Zc:auto                # Deduce Variable Type
            /Zc:char8_t
            /Zc:__cplusplus         # Enable updated __cplusplus macro
            /Zc:externC
            /Zc:externConstexpr     # Enable extern constexpr variables
            /Zc:forScope            # Force Conformance in for Loop Scope
            /Zc:hiddenFriend
            /Zc:implicitNoexcept    # Implicit Exception Specifiers
            /Zc:lambda
            /Zc:noexceptTypes       # C++17 noexcept rules
            /Zc:preprocessor        # Enable preprocessor conformance mode
            /Zc:referenceBinding    # Enforce reference binding rules
            /Zc:rvalueCast          # Enforce type conversion rules
            /Zc:sizedDealloc        # Enable Global Sized Deallocation Functions
            /Zc:strictStrings       # Disable string literal type conversion
            /Zc:ternary             # Enforce conditional operator rules
            /Zc:threadSafeInit      # Thread-safe Local Static Initialization
            /Zc:tlsGuards
            /Zc:trigraphs           # Trigraphs Substitution
            /Zc:wchar_t             # wchar_t Is Native Type

            /external:anglebrackets
            /external:W0

            /W4                     # Warning level
            # /WX                     # Warning level (warnings are errors)

            "$<$<CONFIG:DEBUG>:>"
            "$<$<CONFIG:RELEASE>:/Zc:inline;/JMC-;/Ob3>"

            # Optimization flags
            "$<$<CONFIG:RELEASE>:/O2>"
    )

    target_link_options(
        ${PROJECT_NAME}
        PRIVATE
            /WX # Treat Linker Warnings as Errors

            "$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
            "$<$<CONFIG:RELEASE>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>"
    )
else()
    message(WARNING "MSVC is the only supported compiler. Proceed at your own caution.")
endif()

target_precompile_headers(
    ${PROJECT_NAME}
    PRIVATE
        src/PCH.hpp
)

# ---- Post build ----

if(COPY_BUILD)
    if(SKYRIM64_DATA_PATH)
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${SKYRIM64_DATA_PATH}/SKSE/Plugins
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${SKYRIM64_DATA_PATH}/SKSE/Plugins
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:${PROJECT_NAME}> ${SKYRIM64_DATA_PATH}/SKSE/Plugins
        )
    else()
        message(
            WARNING
            "Variable SKYRIM64_DATA_PATH is empty. Skipping post-build copy command."
        )
    endif()
endif()
