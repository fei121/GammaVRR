#include "refreshtone/model.hpp"

#include <algorithm>
#include <limits>
#include <utility>

namespace refreshtone {
namespace {

[[nodiscard]] constexpr std::int32_t round_divide_by_16(std::int32_t value) noexcept {
    return value >= 0 ? (value + 8) / 16 : (value - 8) / 16;
}

[[nodiscard]] constexpr std::int32_t interpolate_4bit(std::int32_t left, std::int32_t right,
                                                      std::int32_t fraction) noexcept {
    return round_divide_by_16(left * (16 - fraction) + right * fraction);
}

[[nodiscard]] bool expected_sample_count(std::size_t width, std::size_t height,
                                         std::size_t &result) noexcept {
    if (width == 0 || height == 0) {
        return false;
    }
    constexpr auto max = std::numeric_limits<std::size_t>::max();
    if (width > max / height || width * height > max / kChannelCount) {
        return false;
    }
    result = width * height * kChannelCount;
    return true;
}

} // namespace

Model::Model(Config config, Lut lut) noexcept : config_(config), lut_(std::move(lut)) {}

ProcessError Model::process(ImageView input, int frame_level,
                            MutableImageView output) const noexcept {
    if (frame_level < kMinFrameLevel || frame_level > kMaxFrameLevel) {
        return ProcessError::invalid_frame_level;
    }
    if (input.width != output.width || input.height != output.height) {
        return ProcessError::invalid_dimensions;
    }

    std::size_t sample_count = 0;
    if (!expected_sample_count(input.width, input.height, sample_count)) {
        return ProcessError::invalid_dimensions;
    }
    if (input.samples.size() != sample_count) {
        return ProcessError::input_size_mismatch;
    }
    if (output.samples.size() != sample_count) {
        return ProcessError::output_size_mismatch;
    }

    for (const auto sample : input.samples) {
        if (sample > kMaxSampleValue) {
            return ProcessError::sample_out_of_range;
        }
    }

    if (!config_.enabled) {
        std::copy(input.samples.begin(), input.samples.end(), output.samples.begin());
        return ProcessError::none;
    }

    const auto level0 = static_cast<std::size_t>(frame_level >> 4);
    const auto level1 = std::min(level0 + 1, kLevelCount - 1);
    const auto level_fraction = static_cast<std::int32_t>(frame_level & 0x0F);

    for (std::size_t index = 0; index < sample_count; ++index) {
        const auto sample = input.samples[index];
        const auto node0 = static_cast<std::size_t>(sample >> 4);
        const auto node1 = std::min(node0 + 1, kNodeCount - 1);
        const auto node_fraction = static_cast<std::int32_t>(sample & 0x0F);
        const auto channel = index % kChannelCount;

        const auto at_level0 = interpolate_4bit(lut_[channel][level0][node0],
                                                lut_[channel][level0][node1], node_fraction);
        const auto at_level1 = interpolate_4bit(lut_[channel][level1][node0],
                                                lut_[channel][level1][node1], node_fraction);
        const auto offset =
            interpolate_4bit(at_level0, at_level1, level_fraction) - config_.zero_setting;
        const auto corrected = static_cast<std::int32_t>(sample) + offset;
        output.samples[index] = static_cast<std::uint16_t>(
            std::clamp(corrected, 0, static_cast<std::int32_t>(kMaxSampleValue)));
    }

    return ProcessError::none;
}

std::string_view to_string(ProcessError error) noexcept {
    switch (error) {
    case ProcessError::none:
        return "success";
    case ProcessError::invalid_frame_level:
        return "frame level must be in the range 0..127";
    case ProcessError::invalid_dimensions:
        return "image dimensions are invalid or do not match";
    case ProcessError::input_size_mismatch:
        return "input buffer size does not match width * height * 3";
    case ProcessError::output_size_mismatch:
        return "output buffer size does not match width * height * 3";
    case ProcessError::sample_out_of_range:
        return "input contains a sample outside the 12-bit range 0..4095";
    }
    return "unknown processing error";
}

} // namespace refreshtone
