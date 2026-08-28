# External dependencies are installed under .deps/ and otherTools/ by
# install.sh. No third-party source is committed to this repository.
#
# hw-cbmc: the Verilog frontend we link against and the EBMC binary we invoke.
#
# The downloaded dependency builds with its own make (it compiles most of CBMC), so CMake
# drives that script rather than trying to re-express the build. The static
# libraries are the custom command's outputs, so a second `cmake --build`
# does not rebuild them.
#
# See tools/patches/hw-cbmc/README.md for the compatibility patch applied only
# while compiling the dependency.

set(SMART_DEPS_ROOT "${CMAKE_SOURCE_DIR}/.deps" CACHE PATH
    "Locally installed SMART dependencies")
set(HWCBMC_DIR "${SMART_DEPS_ROOT}/hw-cbmc")
set(HWCBMC_CBMC_DIR ${HWCBMC_DIR}/lib/cbmc)

if(NOT EXISTS ${HWCBMC_CBMC_DIR}/src)
    message(FATAL_ERROR
        "hw-cbmc is not installed under ${HWCBMC_DIR}. Run ./install.sh, "
        "or pass -DSMART_DEPS_ROOT=<dependency-prefix>.")
endif()

find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp
    HINTS ${SMART_DEPS_ROOT}/include)
if(NOT NLOHMANN_INCLUDE_DIR)
    message(FATAL_ERROR
        "nlohmann/json.hpp not found. Run ./install.sh, or place it under "
        "${SMART_DEPS_ROOT}/include.")
endif()

# Link order matters and is resolved with --start-group below, but list the
# frontend first so the intent stays readable.
set(HWCBMC_LIBS
    ${HWCBMC_DIR}/src/verilog/verilog.a
    ${HWCBMC_DIR}/src/temporal-logic/temporal-logic.a
    ${HWCBMC_DIR}/src/trans-word-level/trans-word-level.a
    ${HWCBMC_CBMC_DIR}/src/langapi/langapi.a
    ${HWCBMC_CBMC_DIR}/src/solvers/solvers.a
    ${HWCBMC_CBMC_DIR}/src/util/util.a
    ${HWCBMC_CBMC_DIR}/src/big-int/big-int.a
    ${HWCBMC_CBMC_DIR}/src/json/json.a)

set(HWCBMC_EBMC ${HWCBMC_DIR}/src/ebmc/ebmc)

include(ProcessorCount)
ProcessorCount(HWCBMC_JOBS)
if(HWCBMC_JOBS EQUAL 0)
    set(HWCBMC_JOBS 1)
endif()

add_custom_command(
    OUTPUT ${HWCBMC_LIBS} ${HWCBMC_EBMC}
    COMMAND ${CMAKE_SOURCE_DIR}/tools/build-hw-cbmc.sh ${HWCBMC_JOBS}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS
        ${CMAKE_SOURCE_DIR}/tools/build-hw-cbmc.sh
        ${CMAKE_SOURCE_DIR}/tools/apply-hw-cbmc-patches.sh
        ${CMAKE_SOURCE_DIR}/tools/patches/hw-cbmc/0001-verilog-keep-single-attribute-instances.patch
    COMMENT "Building hw-cbmc (ebmc + Verilog frontend) — slow, one-off"
    VERBATIM
    USES_TERMINAL)

add_custom_target(hw-cbmc DEPENDS ${HWCBMC_LIBS} ${HWCBMC_EBMC})

# What consumers link against.
add_library(hwcbmc_frontend INTERFACE)
# These are external headers from pinned hw-cbmc/CBMC. Treat them as system
# includes so project warning policies do not fail on upstream diagnostics.
target_include_directories(hwcbmc_frontend SYSTEM INTERFACE
    ${HWCBMC_DIR}/src
    ${HWCBMC_CBMC_DIR}/src)
# Mandatory: without this our irep ids disagree with the libraries'.
target_compile_definitions(hwcbmc_frontend INTERFACE
    "LOCAL_IREP_IDS=<hw_cbmc_irep_ids.h>")
target_link_libraries(hwcbmc_frontend INTERFACE
    -Wl,--start-group ${HWCBMC_LIBS} -Wl,--end-group)
add_dependencies(hwcbmc_frontend hw-cbmc)

# The checker stage invokes this binary through the runtime dependency path.
