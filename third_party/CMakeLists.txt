# hw-cbmc: the Verilog frontend we link against and the EBMC binary we invoke.
#
# The submodule builds with its own make (it compiles most of CBMC), so CMake
# drives that script rather than trying to re-express the build. The static
# libraries are the custom command's outputs, so a second `cmake --build`
# does not rebuild them.
#
# See docs/PLAN-cpp-single-binary.md §4 WP2A for why this link set, and
# third_party/patches/README.md for the patch applied on the way in.

set(HWCBMC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/hw-cbmc)
set(HWCBMC_CBMC_DIR ${HWCBMC_DIR}/lib/cbmc)

if(NOT EXISTS ${HWCBMC_CBMC_DIR}/src)
    message(FATAL_ERROR
        "third_party/hw-cbmc is not checked out (or its nested cbmc submodule "
        "is missing). Run:\n"
        "    git submodule update --init --recursive")
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
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/build-hw-cbmc.sh ${HWCBMC_JOBS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building hw-cbmc (ebmc + Verilog frontend) — slow, one-off"
    VERBATIM
    USES_TERMINAL)

add_custom_target(hw-cbmc DEPENDS ${HWCBMC_LIBS} ${HWCBMC_EBMC})

# What consumers link against.
add_library(hwcbmc_frontend INTERFACE)
target_include_directories(hwcbmc_frontend INTERFACE
    ${HWCBMC_DIR}/src
    ${HWCBMC_CBMC_DIR}/src)
# Mandatory: without this our irep ids disagree with the libraries'.
target_compile_definitions(hwcbmc_frontend INTERFACE
    "LOCAL_IREP_IDS=<hw_cbmc_irep_ids.h>")
target_link_libraries(hwcbmc_frontend INTERFACE
    -Wl,--start-group ${HWCBMC_LIBS} -Wl,--end-group)
add_dependencies(hwcbmc_frontend hw-cbmc)

# The checker stage (WP7) invokes this binary; export the path.
set(HWCBMC_EBMC ${HWCBMC_EBMC} PARENT_SCOPE)
