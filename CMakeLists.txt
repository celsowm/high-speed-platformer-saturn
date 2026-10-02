# High-speed platformer: a stage-based 2D platformer for the Sega Saturn, built from
# LibSaturn's generic 2D runtime modules and nothing else.
#
# This project is an ordinary external consumer of an installed LibSaturn package. It reaches
# the library only through
#
#     find_package(LibSaturn CONFIG REQUIRED)     ->  LibSaturn::Saturn, LibSaturn::Sim2D
#
# and the helpers that package ships (libsaturn_configure_executable, libsaturn_add_binary,
# libsaturn_add_disc, LIBSATURN_STAGE2D_TOOL). There is no sibling checkout, no
# add_subdirectory(), no private header and no library build-tree path.
#
# Two configurations, chosen by the toolchain:
#   saturn  the SH-2 toolchain file from the LibSaturn prefix: firmware ELF, 0.BIN and a bootable
#           disc (cmake --preset saturn)
#   host    the native compiler: the simulation test, which plays the game's own logic with no
#           video chip, and the boundary test (cmake --preset host)
cmake_minimum_required(VERSION 3.24)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" HSP_VERSION LIMIT_COUNT 1)
string(STRIP "${HSP_VERSION}" HSP_VERSION)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")

project(high_speed_platformer VERSION ${HSP_VERSION} LANGUAGES C CXX)

# 0.1 is the first LibSaturn with the 2D runtime modules, LibSaturn::Sim2D and the stage2d tool.
set(LIBSATURN_MIN_VERSION 0.1)

# The same package is found for both configurations: the host build compiles LibSaturn::Sim2D
# (the hardware-free modules, shipped as sources) and the firmware build links LibSaturn::Saturn.
find_package(LibSaturn ${LIBSATURN_MIN_VERSION} CONFIG REQUIRED)
find_package(Python3 3.9 REQUIRED COMPONENTS Interpreter)

# requirements.txt lists no third-party package: the stage generator and LibSaturn's stage2d tool
# use the standard library only. Python's version is the whole requirement.

include(cmake/StageData.cmake)

if(CMAKE_SYSTEM_NAME STREQUAL "Generic")
    # ---- firmware -------------------------------------------------------
    add_executable(high_speed_platformer
        src/main.c src/game.c src/view.c src/art.c
        ${HSP_GENERATED_SOURCES})
    target_include_directories(high_speed_platformer PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${HSP_GENERATED_DIR}")
    # The runtime is tuned for -O2 (it matches the library); -O2 comes last so it wins over any
    # config default.
    target_compile_options(high_speed_platformer PRIVATE -O2 -Wall -Wextra)
    target_link_libraries(high_speed_platformer PRIVATE LibSaturn::Saturn)
    libsaturn_configure_executable(high_speed_platformer)
    # 0.BIN limit of the Saturn application load area.
    libsaturn_add_binary(high_speed_platformer MAX_BYTES 983040)
    libsaturn_add_disc(high_speed_platformer NAME high_speed_platformer ALL)
    install(FILES
            "$<TARGET_FILE:high_speed_platformer>"
            "$<TARGET_FILE_DIR:high_speed_platformer>/high_speed_platformer.app.bin"
        DESTINATION bin)
    return()
endif()

# ---- host: the simulation test and the boundary test -----------------------
include(CTest)

# game.c and the generated stage are C, and the test is C++. They are compiled as C++ with the
# test, which keeps the game's logic building cleanly under both languages.
set(HSP_SIM_SOURCES src/game.c ${HSP_GENERATED_SOURCES})
set_source_files_properties(${HSP_SIM_SOURCES} PROPERTIES LANGUAGE CXX)
add_executable(test_sim tests/host/test_sim.cpp ${HSP_SIM_SOURCES})
target_compile_features(test_sim PRIVATE cxx_std_20)
target_compile_options(test_sim PRIVATE -Wall -Wextra -O1)
target_include_directories(test_sim PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src" "${HSP_GENERATED_DIR}")
target_link_libraries(test_sim PRIVATE LibSaturn::Sim2D)
add_test(NAME sim COMMAND test_sim)

add_executable(test_boundary tests/host/test_boundary.cpp)
target_compile_features(test_boundary PRIVATE cxx_std_17)
add_test(NAME boundary COMMAND test_boundary "${CMAKE_CURRENT_SOURCE_DIR}" "${LibSaturn_INCLUDE_DIR}")
