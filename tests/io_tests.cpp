#include "gammavrr/io.hpp"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

class TemporaryDirectory {
  public:
    TemporaryDirectory() {
        const auto suffix = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        path_ =
            std::filesystem::temp_directory_path() / ("gammavrr-tests-" + std::to_string(suffix));
        std::filesystem::create_directory(path_);
    }

    ~TemporaryDirectory() { std::filesystem::remove_all(path_); }

    [[nodiscard]] const std::filesystem::path &path() const { return path_; }

  private:
    std::filesystem::path path_;
};

void write_lut(const std::filesystem::path &path, int channel_bias) {
    std::ofstream output(path);
    output << "# generated test LUT\n";
    for (std::size_t level = 0; level < gammavrr::kLevelCount; ++level) {
        for (std::size_t node = 0; node < gammavrr::kNodeCount; ++node) {
            output << static_cast<int>(level) - static_cast<int>(node) + channel_bias << ' ';
        }
        output << "// level " << level << '\n';
    }
}

} // namespace

TEST_CASE("P3 comments are accepted and images round-trip") {
    TemporaryDirectory temporary;
    const auto input_path = temporary.path() / "input.ppm";
    const auto output_path = temporary.path() / "output.ppm";
    {
        std::ofstream file(input_path);
        file << "P3\n# dimensions\n2 1\n4095\n0 1 2\n4093 4094 4095\n";
    }

    const auto image = gammavrr::read_ppm(input_path);
    REQUIRE(image.width == 2);
    REQUIRE(image.height == 1);
    REQUIRE(image.max_value == 4095);
    REQUIRE(image.format == gammavrr::PpmFormat::p3);
    REQUIRE(image.samples == std::vector<std::uint16_t>{0, 1, 2, 4093, 4094, 4095});

    gammavrr::write_ppm(image, output_path);
    const auto round_trip = gammavrr::read_ppm(output_path);
    REQUIRE(round_trip.samples == image.samples);
}

TEST_CASE("12-bit P6 samples use portable big-endian encoding") {
    TemporaryDirectory temporary;
    const auto path = temporary.path() / "image.ppm";
    const gammavrr::Image image{
        1, 1, 4095, gammavrr::PpmFormat::p6, {0x000, 0x123, 0xFFF},
    };

    gammavrr::write_ppm(image, path);
    const auto round_trip = gammavrr::read_ppm(path);
    REQUIRE(round_trip.format == gammavrr::PpmFormat::p6);
    REQUIRE(round_trip.samples == image.samples);
}

TEST_CASE("three signed LUT channels are loaded in level-major order") {
    TemporaryDirectory temporary;
    const auto red = temporary.path() / "r.txt";
    const auto green = temporary.path() / "g.txt";
    const auto blue = temporary.path() / "b.txt";
    write_lut(red, 1);
    write_lut(green, 2);
    write_lut(blue, 3);

    const auto lut = gammavrr::read_lut_files(red, green, blue);
    REQUIRE(lut[0][0][0] == 1);
    REQUIRE(lut[1][3][10] == -5);
    REQUIRE(lut[2][7][255] == -245);
}

TEST_CASE("invalid LUT sizes and out-of-range PPM samples are rejected") {
    TemporaryDirectory temporary;
    const auto short_lut = temporary.path() / "short.txt";
    const auto image_path = temporary.path() / "invalid.ppm";
    {
        std::ofstream output(short_lut);
        output << "1 2 3\n";
    }
    {
        std::ofstream output(image_path);
        output << "P3\n1 1\n4095\n0 0 4096\n";
    }

    REQUIRE_THROWS(gammavrr::read_lut_files(short_lut, short_lut, short_lut));
    REQUIRE_THROWS(gammavrr::read_ppm(image_path));
}
