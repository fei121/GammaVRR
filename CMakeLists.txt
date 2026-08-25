cmake_minimum_required(VERSION 3.20)

project(
    RefreshTone
    VERSION 1.0.0
    DESCRIPTION "Bit-exact reference model for refresh-level-dependent gamma compensation"
    LANGUAGES C CXX
)

include(FetchContent)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(REFRESHTONE_BUILD_CLI "Build the refreshtone command-line tool" ON)
option(REFRESHTONE_BUILD_TESTS "Build the RefreshTone test suite" ON)
option(REFRESHTONE_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

add_library(refreshtone
    src/c_api.cpp
    src/io.cpp
    src/model.cpp
)
add_library(RefreshTone::refreshtone ALIAS refreshtone)
set_target_properties(refreshtone PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_compile_features(refreshtone PUBLIC cxx_std_20)
target_include_directories(refreshtone
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(refreshtone PUBLIC REFRESHTONE_SHARED PRIVATE REFRESHTONE_EXPORTS)
endif()

if(MSVC)
    target_compile_options(refreshtone PRIVATE /W4 /permissive-)
else()
    target_compile_options(refreshtone PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

if(REFRESHTONE_ENABLE_SANITIZERS AND NOT MSVC)
    target_compile_options(refreshtone PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(refreshtone PUBLIC -fsanitize=address,undefined)
endif()

if(REFRESHTONE_BUILD_CLI)
    FetchContent_Declare(
        CLI11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG v2.6.2
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(CLI11)

    add_executable(refreshtone_cli app/main.cpp)
    set_target_properties(refreshtone_cli PROPERTIES OUTPUT_NAME refreshtone)
    if(APPLE)
        set_target_properties(refreshtone_cli PROPERTIES
            INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
    elseif(UNIX)
        set_target_properties(refreshtone_cli PROPERTIES
            INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
    endif()
    target_compile_definitions(refreshtone_cli PRIVATE REFRESHTONE_VERSION="${PROJECT_VERSION}")
    target_link_libraries(refreshtone_cli PRIVATE RefreshTone::refreshtone CLI11::CLI11)
    install(TARGETS refreshtone_cli RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(REFRESHTONE_BUILD_TESTS)
    enable_testing()
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.15.3
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(Catch2)

    add_executable(refreshtone_tests
        tests/c_api_tests.cpp
        tests/io_tests.cpp
        tests/model_tests.cpp
    )
    target_link_libraries(refreshtone_tests PRIVATE RefreshTone::refreshtone Catch2::Catch2WithMain)

    add_library(refreshtone_c_header_compile OBJECT tests/c_header_compile.c)
    target_include_directories(refreshtone_c_header_compile PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

    include(Catch)
    catch_discover_tests(refreshtone_tests)
endif()

install(TARGETS refreshtone
    EXPORT RefreshToneTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT RefreshToneTargets
    FILE RefreshToneTargets.cmake
    NAMESPACE RefreshTone::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RefreshTone
)

configure_package_config_file(
    cmake/RefreshToneConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/RefreshToneConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RefreshTone
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/RefreshToneConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/RefreshToneConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/RefreshToneConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RefreshTone
)
