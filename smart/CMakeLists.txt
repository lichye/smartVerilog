# The SMART build. Mirrors smart/Makefile until WP8 retires it; the flex
# prefix and the generated file names must match exactly or the VCD parser
# symbols will not link (plan gotcha 8).

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)
find_package(Threads REQUIRED)

# libcvc5: the solver behind SyGuS (WP6) and the MUS/minimiser (WP5). Point
# CVC5_ROOT at an install prefix (otherTools/cvc5 by default, which is what
# install.sh builds).
set(CVC5_ROOT "${CMAKE_SOURCE_DIR}/otherTools/cvc5" CACHE PATH
    "cvc5 install prefix (headers + static libs)")
find_path(CVC5_INCLUDE_DIR cvc5/cvc5.h HINTS ${CVC5_ROOT}/include)
find_library(CVC5_LIBRARY cvc5 HINTS ${CVC5_ROOT}/lib64 ${CVC5_ROOT}/lib)
find_library(CVC5_PARSER_LIBRARY cvc5parser HINTS ${CVC5_ROOT}/lib64 ${CVC5_ROOT}/lib)
if(NOT CVC5_INCLUDE_DIR OR NOT CVC5_LIBRARY OR NOT CVC5_PARSER_LIBRARY)
    message(FATAL_ERROR
        "libcvc5 not found under ${CVC5_ROOT}. Build it once with "
        "tools/build-cvc5.sh, or pass -DCVC5_ROOT=<prefix>.")
endif()

add_library(cvc5_api INTERFACE)
target_include_directories(cvc5_api INTERFACE ${CVC5_INCLUDE_DIR})
# The static build pulls its own dependencies in; list them explicitly since
# the install ships no CMake usage requirements for them.
foreach(dep cadical picpolyxx picpoly gmpxx gmp)
    find_library(CVC5_DEP_${dep} ${dep} HINTS ${CVC5_ROOT}/lib64 ${CVC5_ROOT}/lib)
    if(CVC5_DEP_${dep})
        list(APPEND CVC5_DEPS ${CVC5_DEP_${dep}})
    endif()
endforeach()
target_link_libraries(cvc5_api INTERFACE
    ${CVC5_PARSER_LIBRARY} ${CVC5_LIBRARY} ${CVC5_DEPS})

set(SMART_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SMART_GENERATED ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${SMART_GENERATED})

# --- VCD parser (flex/bison) ------------------------------------------------
# `-P VCDParser` and the .hpp names are load-bearing: VCDScanner.l includes
# "VCDParser.hpp" and the grammar declares `%define api.prefix {VCDParser}`.
bison_target(VCDParser
    ${SMART_SRC}/parser/VCDParser.ypp
    ${SMART_GENERATED}/VCDParser.cpp
    DEFINES_FILE ${SMART_GENERATED}/VCDParser.hpp
    COMPILE_FLAGS "-v")

flex_target(VCDScanner
    ${SMART_SRC}/parser/VCDScanner.l
    ${SMART_GENERATED}/VCDScanner.cpp
    DEFINES_FILE ${SMART_GENERATED}/VCDScanner.hpp
    COMPILE_FLAGS "-P VCDParser")

add_flex_bison_dependency(VCDScanner VCDParser)

# --- the reusable core ------------------------------------------------------
file(GLOB SMART_CORE_SOURCES
    ${SMART_SRC}/parser/*.cpp
    ${SMART_SRC}/tracer/*.cpp
    ${SMART_SRC}/helper/*.cpp
    ${SMART_SRC}/sygus/*.cpp)

add_library(smart_core STATIC
    ${SMART_CORE_SOURCES}
    ${SMART_SRC}/smart.cpp          # runSmartBlock(): one block, one process
    ${BISON_VCDParser_OUTPUTS}
    ${FLEX_VCDScanner_OUTPUTS})

target_include_directories(smart_core PUBLIC
    ${SMART_SRC}
    ${SMART_SRC}/parser
    ${SMART_SRC}/tracer
    ${SMART_SRC}/helper
    ${SMART_SRC}/sygus
    ${SMART_GENERATED}
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party)  # vendored nlohmann/json.hpp

target_link_libraries(smart_core PUBLIC Threads::Threads cvc5_api)

# --- frontend ---------------------------------------------------------------
# SVModule is the frozen regex ORACLE and the ModuleInfo contract (WP2-regex);
# HwcbmcFrontend (WP2) fills the same struct from hw-cbmc.
add_library(smart_frontend STATIC ${SMART_SRC}/frontend/SVModule.cpp)
target_include_directories(smart_frontend PUBLIC ${SMART_SRC}/frontend)

# --- executables ------------------------------------------------------------
# smart.out: the single-block worker, kept as its own binary for debugging.
# The block logic itself lives in smart_core as runSmartBlock().
add_executable(smart_out ${SMART_SRC}/smart_main.cpp)
set_target_properties(smart_out PROPERTIES OUTPUT_NAME smart.out)
target_link_libraries(smart_out PRIVATE smart_core smart_pipeline)

# Standalone frontend dump + spec tests (WP2-regex evidence, kept green).
add_executable(frontend_dump ${SMART_SRC}/frontend/frontend_dump.cpp)
target_link_libraries(frontend_dump PRIVATE smart_frontend)

add_executable(test_svmodule ${SMART_SRC}/frontend/test_svmodule.cpp)
target_link_libraries(test_svmodule PRIVATE smart_frontend)

# --- the hw-cbmc-backed frontend (WP2) --------------------------------------
# hw-cbmc itself is built once by install.sh / third_party/build-hw-cbmc.sh;
# the custom target only re-runs if its libraries are missing, so linking
# against it costs a plain `all` build nothing.
add_library(smart_hwcbmc_frontend STATIC
    ${SMART_SRC}/frontend/HwcbmcFrontend.cpp)
target_link_libraries(smart_hwcbmc_frontend PUBLIC smart_frontend hwcbmc_frontend)

add_executable(hwcbmc_dump ${SMART_SRC}/frontend/hwcbmc_dump.cpp)
target_link_libraries(hwcbmc_dump PRIVATE smart_hwcbmc_frontend)

# --- simgen (WP3): testbench generation + iverilog/vvp ----------------------
add_library(smart_simgen STATIC ${SMART_SRC}/simgen/Harness.cpp)
target_include_directories(smart_simgen PUBLIC ${SMART_SRC}/simgen)
target_link_libraries(smart_simgen PUBLIC smart_frontend)

add_executable(simgen_run ${SMART_SRC}/simgen/simgen_run.cpp)
target_link_libraries(simgen_run PRIVATE smart_simgen smart_hwcbmc_frontend)

# --- pipeline (WP4): options/config, workdir, orchestration ----------------
# Options is separate from the rest of the pipeline because smart_core needs it
# too (a block reads the same config), and smart_core must not depend on the
# orchestration it is orchestrated by.
add_library(smart_pipeline STATIC ${SMART_SRC}/pipeline/Options.cpp)
target_include_directories(smart_pipeline PUBLIC
    ${SMART_SRC}/pipeline
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

target_link_libraries(smart_core PUBLIC smart_pipeline)

add_executable(test_options ${SMART_SRC}/pipeline/test_options.cpp)
target_link_libraries(test_options PRIVATE smart_pipeline)

# --- mus (WP5): MSA/MUS + the assertion minimiser, on libcvc5 ---------------
add_library(smart_mus STATIC ${SMART_SRC}/mus/Mus.cpp)
target_include_directories(smart_mus PUBLIC ${SMART_SRC}/mus)
target_link_libraries(smart_mus PUBLIC cvc5_api)

add_executable(test_mus ${SMART_SRC}/mus/test_mus.cpp)
target_link_libraries(test_mus PRIVATE smart_mus)

# --- emit (WP7): final EBMC gate + the file the user gets -------------------
add_library(smart_emit STATIC ${SMART_SRC}/emit/AssertionWriter.cpp)
target_include_directories(smart_emit PUBLIC ${SMART_SRC}/emit)

add_executable(test_emit ${SMART_SRC}/emit/test_emit.cpp)
target_link_libraries(test_emit PRIVATE smart_emit)

# --- the smart binary -------------------------------------------------------
# THE product. It belongs in `all`: a plain `cmake --build build` must produce
# the tool, not everything except the tool.
add_executable(smart
    ${SMART_SRC}/main.cpp
    ${SMART_SRC}/pipeline/Pipeline.cpp
    ${SMART_SRC}/pipeline/BlockRunner.cpp
    ${SMART_SRC}/pipeline/Blocks.cpp
    ${SMART_SRC}/pipeline/WorkDir.cpp)
target_link_libraries(smart PRIVATE
    smart_core smart_pipeline smart_simgen smart_emit smart_mus
    smart_hwcbmc_frontend)

# WP3 acceptance instrument: what the pipeline's OWN trace loader sees in a
# VCD. Builds without hw-cbmc, so it stays in `all`.
add_executable(vcd_signals ${SMART_SRC}/simgen/vcd_signals.cpp)
target_link_libraries(vcd_signals PRIVATE smart_core)

# WP2A link spike: proves the hw-cbmc frontend links and exposes what
# ModuleInfo needs. Kept as the minimal reproducer if linking ever breaks.
add_executable(hwcbmc_spike EXCLUDE_FROM_ALL ${SMART_SRC}/frontend/hwcbmc_spike.cpp)
target_link_libraries(hwcbmc_spike PRIVATE hwcbmc_frontend)

enable_testing()
# These read benchmark sources by relative path, so they must run from the
# repository root, not from the build tree.
add_test(NAME svmodule COMMAND test_svmodule
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME options COMMAND test_options
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME emit COMMAND test_emit)
add_test(NAME mus COMMAND test_mus)
