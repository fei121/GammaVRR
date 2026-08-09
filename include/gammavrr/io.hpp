#ifndef GAMMAVRR_IO_HPP
#define GAMMAVRR_IO_HPP

#include "gammavrr/model.hpp"

#include <cstdint>
#include <filesystem>
#include <vector>

namespace gammavrr {

enum class PpmFormat {
    p3,
    p6,
};

struct Image {
    std::size_t width{};
    std::size_t height{};
    std::uint16_t max_value{};
    PpmFormat format{PpmFormat::p3};
    std::vector<std::uint16_t> samples;
};

[[nodiscard]] Lut read_lut_files(const std::filesystem::path &red_path,
                                 const std::filesystem::path &green_path,
                                 const std::filesystem::path &blue_path);

[[nodiscard]] Image read_ppm(const std::filesystem::path &path);

void write_ppm(const Image &image, const std::filesystem::path &path);

} // namespace gammavrr

#endif
