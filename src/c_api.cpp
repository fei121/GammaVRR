#include "refreshtone/c_api.h"

#include "refreshtone/model.hpp"

#include <limits>
#include <span>
#include <utility>

namespace {

[[nodiscard]] refreshtone_status map_error(refreshtone::ProcessError error) noexcept {
    using refreshtone::ProcessError;
    switch (error) {
    case ProcessError::none:
        return REFRESHTONE_STATUS_OK;
    case ProcessError::invalid_frame_level:
        return REFRESHTONE_STATUS_INVALID_FRAME_LEVEL;
    case ProcessError::invalid_dimensions:
        return REFRESHTONE_STATUS_INVALID_DIMENSIONS;
    case ProcessError::input_size_mismatch:
    case ProcessError::output_size_mismatch:
        return REFRESHTONE_STATUS_BUFFER_SIZE_MISMATCH;
    case ProcessError::sample_out_of_range:
        return REFRESHTONE_STATUS_SAMPLE_OUT_OF_RANGE;
    }
    return REFRESHTONE_STATUS_INTERNAL_ERROR;
}

} // namespace

extern "C" refreshtone_status refreshtone_process(const uint16_t *input, size_t input_sample_count,
                                            uint16_t *output, size_t output_sample_count,
                                            size_t width, size_t height, const int16_t *lut_r,
                                            const int16_t *lut_g, const int16_t *lut_b,
                                            const refreshtone_params *params) {
    if (input == nullptr || output == nullptr || lut_r == nullptr || lut_g == nullptr ||
        lut_b == nullptr || params == nullptr) {
        return REFRESHTONE_STATUS_INVALID_ARGUMENT;
    }

    try {
        refreshtone::Lut lut{};
        const int16_t *channels[refreshtone::kChannelCount] = {lut_r, lut_g, lut_b};
        for (std::size_t channel = 0; channel < refreshtone::kChannelCount; ++channel) {
            for (std::size_t level = 0; level < refreshtone::kLevelCount; ++level) {
                for (std::size_t node = 0; node < refreshtone::kNodeCount; ++node) {
                    lut[channel][level][node] =
                        channels[channel][level * refreshtone::kNodeCount + node];
                }
            }
        }

        const refreshtone::Config config{
            params->enabled != 0,
            params->zero_setting,
        };
        const refreshtone::Model model(config, std::move(lut));
        const auto error =
            model.process({{input, input_sample_count}, width, height}, params->frame_level,
                          {{output, output_sample_count}, width, height});
        return map_error(error);
    } catch (...) {
        return REFRESHTONE_STATUS_INTERNAL_ERROR;
    }
}

extern "C" const char *refreshtone_status_string(refreshtone_status status) {
    switch (status) {
    case REFRESHTONE_STATUS_OK:
        return "success";
    case REFRESHTONE_STATUS_INVALID_ARGUMENT:
        return "a required pointer argument is null";
    case REFRESHTONE_STATUS_INVALID_FRAME_LEVEL:
        return "frame level must be in the range 0..127";
    case REFRESHTONE_STATUS_INVALID_DIMENSIONS:
        return "image dimensions are invalid or do not match";
    case REFRESHTONE_STATUS_BUFFER_SIZE_MISMATCH:
        return "buffer size does not match width * height * 3";
    case REFRESHTONE_STATUS_SAMPLE_OUT_OF_RANGE:
        return "input contains a sample outside the 12-bit range 0..4095";
    case REFRESHTONE_STATUS_INTERNAL_ERROR:
        return "internal error";
    }
    return "unknown status";
}
