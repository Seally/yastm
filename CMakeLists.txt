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

set(SKYRIM_VERSION "AE2" CACHE STRING "Sets the DLL target compile version.")

set_property(CACHE SKYRIM_VERSION PROPERTY STRINGS "AE" "AE2" "SE" "VR")

# ---- Project ----

project(
    YASTM
    VERSION 1.11.0
    LANGUAGES CXX
)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(
        FATAL_ERROR
            "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
    )
endif()

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


if(SKYRIM_VERSION STREQUAL "SE")
    add_subdirectory("extern/CommonLibSSE_SE" CommonLibSSE)
elseif(SKYRIM_VERSION STREQUAL "AE")
    add_subdirectory("extern/CommonLibSSE_AE" CommonLibSSE)
elseif(SKYRIM_VERSION STREQUAL "AE2")
    add_subdirectory("extern/CommonLibSSE_AE2" CommonLibSSE)
elseif(SKYRIM_VERSION STREQUAL "VR")
    if (BUILD_SKYRIMVR)
        add_subdirectory("extern/CommonLibVR" CommonLibVR)
    else()
        add_subdirectory("extern/CommonLibVR" CommonLibSSE)
    endif()
else()
    message(FATAL_ERROR "Unknown Skyrim version: ${SKYRIM_VERSION}")
endif()


find_package(spdlog REQUIRED CONFIG)
find_package(tomlplusplus REQUIRED)

# ---- Add source files ----

set(SOURCES
    src/global.hpp
    src/main.cpp
    src/PCH.hpp
    src/chargeitemfix.hpp
    src/chargeitemfix.cpp
    src/enchantitemfix.hpp
    src/enchantitemfix.cpp
    src/expectedbytes.hpp
    src/messages.hpp
    src/offsets.hpp
    src/SoulSize.hpp
    src/SoulValue.hpp
    src/trampoline.hpp
    src/trapsoulfix.hpp
    src/trapsoulfix.cpp
    src/config/ConcreteSoulGemGroup.hpp
    src/config/ConcreteSoulGemGroup.cpp
    src/config/ConfigKey.hpp
    src/config/configutilities.hpp
    src/config/DLLDependencyKey.hpp
    src/config/Form.hpp
    src/config/FormError.hpp
    src/config/FormError.cpp
    src/config/FormId.hpp
    src/config/FormId.cpp
    src/config/FormLocator.hpp
    src/config/GlobalVarForm.hpp
    src/config/LoadPriority.hpp
    src/config/ParseError.hpp
    src/config/SoulGemGroup.hpp
    src/config/SoulGemGroup.cpp
    src/config/SoulGemMap.hpp
    src/config/SoulGemMap.cpp
    src/config/SpecificationError.hpp
    src/config/SpecificationError.cpp
    src/config/YASTMConfig.hpp
    src/config/YASTMConfig.cpp
    src/formatters/TESForm.hpp
    src/formatters/TESSoulGem.hpp
    src/fsutils/FSUtils.hpp
    src/fsutils/FSUtils.cpp
    src/fsutils/internal/Config.hpp
    src/fsutils/internal/Config.cpp
    src/fsutils/internal/ConfigManager.hpp
    src/fsutils/internal/ConfigManager.cpp
    src/trapsoul/SearchResult.hpp
    src/trapsoul/SoulTrapData.hpp
    src/trapsoul/SoulTrapData.cpp
    src/trapsoul/trapsoul.hpp
    src/trapsoul/trapsoul.cpp
    src/trapsoul/types.hpp
    src/trapsoul/Victim.hpp
    src/trapsoul/Victim.cpp
    src/utilities/algorithms.hpp
    src/utilities/assembly.hpp
    src/utilities/containerutils.hpp
    src/utilities/EnumArray.hpp
    src/utilities/formidutils.hpp
    src/utilities/FormType.hpp
    src/utilities/misc.hpp
    src/utilities/misc.cpp
    src/utilities/native.hpp
    src/utilities/PapyrusFunctionRegistry.hpp
    src/utilities/printerror.hpp
    src/utilities/printerror.cpp
    src/utilities/stringutils.hpp
    src/utilities/Timer.hpp
    src/yastmutils/YASTMUtils.hpp
    src/yastmutils/YASTMUtils.cpp
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
        cxx_std_23
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (SKYRIM_VERSION STREQUAL "VR" AND BUILD_SKYRIMVR)
    target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
            CommonLibVR::CommonLibVR
            spdlog::spdlog
            fmt::fmt
            tomlplusplus::tomlplusplus
    )
else()
    target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
            CommonLibSSE::CommonLibSSE
            spdlog::spdlog
            fmt::fmt
            tomlplusplus::tomlplusplus
    )
endif()

if(MSVC)
    target_compile_options(
        ${PROJECT_NAME}
        PRIVATE
            /sdl                    # Enable Additional Security Checks
            /utf-8                  # Set Source and Executable character sets to UTF-8
            /Zi                     # Debug Information Format

            /permissive-            # Standards conformance
            /Zc:preprocessor        # Enable preprocessor conformance mode
            /wd4200                 # nonstandard extension used : zero-sized array in struct/union

            /external:anglebrackets
            /external:W0

            /W4                     # Warning level
            # /WX                     # Warning level (warnings are errors)

            "$<$<CONFIG:Debug>:>"
            "$<$<CONFIG:Release,RelWithDebInfo>:/Zc:inline;/JMC-;/O2;/Ob3>"
    )

    target_link_options(
        ${PROJECT_NAME}
        PRIVATE
            /WX # Treat Linker Warnings as Errors

            "$<$<CONFIG:Debug>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
            "$<$<CONFIG:Release,RelWithDebInfo>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>"
    )
else()
    message(WARNING "MSVC is the only supported compiler. Proceed at your own caution.")
endif()

target_precompile_headers(
    ${PROJECT_NAME}
    PRIVATE
        src/PCH.hpp
)

if (SKYRIM_VERSION STREQUAL "AE")
    target_compile_definitions(${PROJECT_NAME} PRIVATE SKYRIM_VERSION_AE)
elseif(SKYRIM_VERSION STREQUAL "AE2")
    target_compile_definitions(${PROJECT_NAME} PRIVATE SKYRIM_VERSION_AE2)
elseif(SKYRIM_VERSION STREQUAL "SE")
    target_compile_definitions(${PROJECT_NAME} PRIVATE SKYRIM_VERSION_SE)
elseif(SKYRIM_VERSION STREQUAL "VR")
    target_compile_definitions(${PROJECT_NAME} PRIVATE SKYRIM_VERSION_VR SKYRIMVR)
else()
    message(FATAL_ERROR "Unknown Skyrim version: ${SKYRIM_VERSION}")
endif()

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
