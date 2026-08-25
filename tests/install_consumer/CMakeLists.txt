cmake_minimum_required(VERSION 3.20)
project(RefreshToneInstallConsumer LANGUAGES CXX)

find_package(RefreshTone 1 CONFIG REQUIRED)

add_executable(refreshtone_install_consumer main.cpp)
target_compile_features(refreshtone_install_consumer PRIVATE cxx_std_20)
target_link_libraries(refreshtone_install_consumer PRIVATE RefreshTone::refreshtone)
