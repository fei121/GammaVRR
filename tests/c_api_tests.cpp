#include "gammavrr/c_api.h"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>

TEST_CASE("C interface processes RGB samples and supports in-place operation") {
    std::array<std::int16_t, 8 * 256> red{};
    std::array<std::int16_t, 8 * 256> green{};
    std::array<std::int16_t, 8 * 256> blue{};
    red.fill(10);
    green.fill(-20);
    blue.fill(30);
    std::array<std::uint16_t, 3> samples{100, 100, 100};
    const gammavrr_params params{1, 5, 73};

    const auto status =
        gammavrr_process(samples.data(), samples.size(), samples.data(), samples.size(), 1, 1,
                         red.data(), green.data(), blue.data(), &params);

    REQUIRE(status == GAMMAVRR_STATUS_OK);
    REQUIRE(samples == std::array<std::uint16_t, 3>{105, 75, 125});
}

TEST_CASE("C interface reports invalid arguments and buffer sizes") {
    std::array<std::int16_t, 8 * 256> lut{};
    std::array<std::uint16_t, 3> input{};
    std::array<std::uint16_t, 3> output{};
    const gammavrr_params params{1, 0, 0};

    REQUIRE(gammavrr_process(nullptr, 3, output.data(), 3, 1, 1, lut.data(), lut.data(), lut.data(),
                             &params) == GAMMAVRR_STATUS_INVALID_ARGUMENT);
    REQUIRE(gammavrr_process(input.data(), 2, output.data(), 3, 1, 1, lut.data(), lut.data(),
                             lut.data(), &params) == GAMMAVRR_STATUS_BUFFER_SIZE_MISMATCH);
    REQUIRE(std::string(gammavrr_status_string(GAMMAVRR_STATUS_OK)) == "success");
}
