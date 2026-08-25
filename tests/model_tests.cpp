#include "refreshtone/model.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace {

[[nodiscard]] std::int32_t reference_interp(std::int32_t left, std::int32_t right,
                                            std::int32_t fraction) {
    const auto numerator = left * (16 - fraction) + right * fraction;
    return static_cast<std::int32_t>(std::round(static_cast<double>(numerator) / 16.0));
}

[[nodiscard]] std::uint16_t reference_sample(const refreshtone::Lut &lut, std::uint16_t sample,
                                             int frame_level, std::size_t channel,
                                             std::int32_t zero_setting) {
    const auto node0 = static_cast<std::size_t>(sample >> 4);
    const auto node1 = std::min(node0 + 1, refreshtone::kNodeCount - 1);
    const auto node_fraction = static_cast<std::int32_t>(sample & 0x0F);
    const auto level0 = static_cast<std::size_t>(frame_level >> 4);
    const auto level1 = std::min(level0 + 1, refreshtone::kLevelCount - 1);
    const auto level_fraction = static_cast<std::int32_t>(frame_level & 0x0F);

    const auto at_level0 =
        reference_interp(lut[channel][level0][node0], lut[channel][level0][node1], node_fraction);
    const auto at_level1 =
        reference_interp(lut[channel][level1][node0], lut[channel][level1][node1], node_fraction);
    const auto offset = reference_interp(at_level0, at_level1, level_fraction) - zero_setting;
    return static_cast<std::uint16_t>(
        std::clamp(static_cast<std::int32_t>(sample) + offset, 0,
                   static_cast<std::int32_t>(refreshtone::kMaxSampleValue)));
}

[[nodiscard]] refreshtone::Lut varied_lut() {
    refreshtone::Lut lut{};
    for (std::size_t channel = 0; channel < refreshtone::kChannelCount; ++channel) {
        for (std::size_t level = 0; level < refreshtone::kLevelCount; ++level) {
            for (std::size_t node = 0; node < refreshtone::kNodeCount; ++node) {
                const auto pattern = static_cast<int>((node * 5 + level * 37 + channel * 11) % 401);
                lut[channel][level][node] = static_cast<std::int16_t>(pattern - 200);
            }
        }
    }
    return lut;
}

} // namespace

TEST_CASE("constant offsets are applied per RGB channel") {
    refreshtone::Lut lut{};
    for (auto &level : lut[0]) {
        level.fill(10);
    }
    for (auto &level : lut[1]) {
        level.fill(-20);
    }
    for (auto &level : lut[2]) {
        level.fill(30);
    }

    const refreshtone::Model model({true, 5}, lut);
    const std::vector<std::uint16_t> input{100, 100, 100};
    std::vector<std::uint16_t> output(3);
    const auto error = model.process({input, 1, 1}, 73, {output, 1, 1});

    REQUIRE(error == refreshtone::ProcessError::none);
    REQUIRE(output == std::vector<std::uint16_t>{105, 75, 125});
}

TEST_CASE("two-stage interpolation matches the fixed golden vector") {
    refreshtone::Lut lut{};
    lut[0][1][1] = 0;
    lut[0][1][2] = 16;
    lut[0][2][1] = 16;
    lut[0][2][2] = 32;

    lut[1][1][1] = 0;
    lut[1][1][2] = -16;
    lut[1][2][1] = -16;
    lut[1][2][2] = -32;

    lut[2][1][255] = 10;
    lut[2][2][255] = 30;

    const refreshtone::Model model({true, 3}, lut);
    const std::vector<std::uint16_t> input{24, 24, 4095};
    std::vector<std::uint16_t> output(3);

    REQUIRE(model.process({input, 1, 1}, 24, {output, 1, 1}) == refreshtone::ProcessError::none);
    REQUIRE(output == std::vector<std::uint16_t>{37, 5, 4095});
}

TEST_CASE("interpolation rounds exact halves away from zero") {
    refreshtone::Lut lut{};
    lut[0][0][0] = 0;
    lut[0][0][1] = 1;
    lut[1][0][0] = 0;
    lut[1][0][1] = -1;

    const refreshtone::Model model({}, lut);
    const std::vector<std::uint16_t> input{8, 8, 8};
    std::vector<std::uint16_t> output(3);

    REQUIRE(model.process({input, 1, 1}, 0, {output, 1, 1}) == refreshtone::ProcessError::none);
    REQUIRE(output[0] == 9);
    REQUIRE(output[1] == 7);
    REQUIRE(output[2] == 8);
}

TEST_CASE("output is saturated to the 12-bit range") {
    refreshtone::Lut lut{};
    for (auto &level : lut[0]) {
        level.fill(-100);
    }
    for (auto &level : lut[1]) {
        level.fill(100);
    }

    const refreshtone::Model model({}, lut);
    const std::vector<std::uint16_t> input{20, 4070, 4095};
    std::vector<std::uint16_t> output(3);

    REQUIRE(model.process({input, 1, 1}, 0, {output, 1, 1}) == refreshtone::ProcessError::none);
    REQUIRE(output == std::vector<std::uint16_t>{0, 4095, 4095});
}

TEST_CASE("disabled model validates then copies input and supports in-place processing") {
    const refreshtone::Model model({false, 123}, varied_lut());
    std::vector<std::uint16_t> samples{0, 16, 4095, 100, 200, 300};
    const auto original = samples;

    REQUIRE(model.process({samples, 2, 1}, 127, {samples, 2, 1}) == refreshtone::ProcessError::none);
    REQUIRE(samples == original);
}

TEST_CASE("validation errors do not modify output") {
    const refreshtone::Model model({}, varied_lut());
    const std::vector<std::uint16_t> invalid_input{0, 4096, 0};
    std::vector<std::uint16_t> output{7, 7, 7};

    REQUIRE(model.process({invalid_input, 1, 1}, 0, {output, 1, 1}) ==
            refreshtone::ProcessError::sample_out_of_range);
    REQUIRE(output == std::vector<std::uint16_t>{7, 7, 7});
    REQUIRE(model.process({invalid_input, 1, 1}, -1, {output, 1, 1}) ==
            refreshtone::ProcessError::invalid_frame_level);
}

TEST_CASE("buffer and dimension errors are reported") {
    const refreshtone::Model model({}, {});
    const std::vector<std::uint16_t> input(3);
    std::vector<std::uint16_t> output(3);

    REQUIRE(model.process({input, 0, 1}, 0, {output, 0, 1}) ==
            refreshtone::ProcessError::invalid_dimensions);
    REQUIRE(model.process({input, 1, 1}, 0, {output, 2, 1}) ==
            refreshtone::ProcessError::invalid_dimensions);
    REQUIRE(model.process({std::span<const std::uint16_t>(input.data(), 2), 1, 1}, 0,
                          {output, 1, 1}) == refreshtone::ProcessError::input_size_mismatch);
}

TEST_CASE("bit-exact model matches an independent floating-point oracle exhaustively") {
    const auto lut = varied_lut();
    constexpr std::int32_t zero_setting = -13;
    const refreshtone::Model model({true, zero_setting}, lut);

    std::vector<std::uint16_t> input(refreshtone::kNodeCount * 16 * refreshtone::kChannelCount);
    std::vector<std::uint16_t> output(input.size());
    for (std::size_t sample = 0; sample <= refreshtone::kMaxSampleValue; ++sample) {
        for (std::size_t channel = 0; channel < refreshtone::kChannelCount; ++channel) {
            input[sample * refreshtone::kChannelCount + channel] = static_cast<std::uint16_t>(sample);
        }
    }

    for (int frame_level = refreshtone::kMinFrameLevel; frame_level <= refreshtone::kMaxFrameLevel;
         ++frame_level) {
        REQUIRE(model.process({input, refreshtone::kNodeCount * 16, 1}, frame_level,
                              {output, refreshtone::kNodeCount * 16, 1}) ==
                refreshtone::ProcessError::none);

        for (std::size_t sample = 0; sample <= refreshtone::kMaxSampleValue; ++sample) {
            for (std::size_t channel = 0; channel < refreshtone::kChannelCount; ++channel) {
                REQUIRE(output[sample * refreshtone::kChannelCount + channel] ==
                        reference_sample(lut, static_cast<std::uint16_t>(sample), frame_level,
                                         channel, zero_setting));
            }
        }
    }
}
