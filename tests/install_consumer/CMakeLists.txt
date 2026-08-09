cmake_minimum_required(VERSION 3.20)
project(GammaVRRInstallConsumer LANGUAGES CXX)

find_package(GammaVRR 1 CONFIG REQUIRED)

add_executable(gammavrr_install_consumer main.cpp)
target_compile_features(gammavrr_install_consumer PRIVATE cxx_std_20)
target_link_libraries(gammavrr_install_consumer PRIVATE GammaVRR::gammavrr)
