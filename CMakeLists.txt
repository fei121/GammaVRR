cmake_minimum_required(VERSION 3.20)

project(
    GammaVRR
    VERSION 1.0.0
    DESCRIPTION "Bit-exact reference model for refresh-level-dependent gamma compensation"
    LANGUAGES C CXX
)

include(FetchContent)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(GAMMAVRR_BUILD_CLI "Build the gammavrr command-line tool" ON)
option(GAMMAVRR_BUILD_TESTS "Build the GammaVRR test suite" ON)
option(GAMMAVRR_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

add_library(gammavrr
    src/c_api.cpp
    src/io.cpp
    src/model.cpp
)
add_library(GammaVRR::gammavrr ALIAS gammavrr)
set_target_properties(gammavrr PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_compile_features(gammavrr PUBLIC cxx_std_20)
target_include_directories(gammavrr
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(gammavrr PUBLIC GAMMAVRR_SHARED PRIVATE GAMMAVRR_EXPORTS)
endif()

if(MSVC)
    target_compile_options(gammavrr PRIVATE /W4 /permissive-)
else()
    target_compile_options(gammavrr PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

if(GAMMAVRR_ENABLE_SANITIZERS AND NOT MSVC)
    target_compile_options(gammavrr PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(gammavrr PUBLIC -fsanitize=address,undefined)
endif()

if(GAMMAVRR_BUILD_CLI)
    FetchContent_Declare(
        CLI11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG v2.6.2
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(CLI11)

    add_executable(gammavrr_cli app/main.cpp)
    set_target_properties(gammavrr_cli PROPERTIES OUTPUT_NAME gammavrr)
    if(APPLE)
        set_target_properties(gammavrr_cli PROPERTIES
            INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
    elseif(UNIX)
        set_target_properties(gammavrr_cli PROPERTIES
            INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
    endif()
    target_compile_definitions(gammavrr_cli PRIVATE GAMMAVRR_VERSION="${PROJECT_VERSION}")
    target_link_libraries(gammavrr_cli PRIVATE GammaVRR::gammavrr CLI11::CLI11)
    install(TARGETS gammavrr_cli RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(GAMMAVRR_BUILD_TESTS)
    enable_testing()
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.15.3
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(Catch2)

    add_executable(gammavrr_tests
        tests/c_api_tests.cpp
        tests/io_tests.cpp
        tests/model_tests.cpp
    )
    target_link_libraries(gammavrr_tests PRIVATE GammaVRR::gammavrr Catch2::Catch2WithMain)

    add_library(gammavrr_c_header_compile OBJECT tests/c_header_compile.c)
    target_include_directories(gammavrr_c_header_compile PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

    include(Catch)
    catch_discover_tests(gammavrr_tests)
endif()

install(TARGETS gammavrr
    EXPORT GammaVRRTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT GammaVRRTargets
    FILE GammaVRRTargets.cmake
    NAMESPACE GammaVRR::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GammaVRR
)

configure_package_config_file(
    cmake/GammaVRRConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/GammaVRRConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GammaVRR
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/GammaVRRConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/GammaVRRConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/GammaVRRConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GammaVRR
)
