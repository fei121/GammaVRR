#include "gammavrr/c_api.h"

#include "gammavrr/model.hpp"

#include <limits>
#include <span>
#include <utility>

namespace {

[[nodiscard]] gammavrr_status map_error(gammavrr::ProcessError error) noexcept {
    using gammavrr::ProcessError;
    switch (error) {
    case ProcessError::none:
        return GAMMAVRR_STATUS_OK;
    case ProcessError::invalid_frame_level:
        return GAMMAVRR_STATUS_INVALID_FRAME_LEVEL;
    case ProcessError::invalid_dimensions:
        return GAMMAVRR_STATUS_INVALID_DIMENSIONS;
    case ProcessError::input_size_mismatch:
    case ProcessError::output_size_mismatch:
        return GAMMAVRR_STATUS_BUFFER_SIZE_MISMATCH;
    case ProcessError::sample_out_of_range:
        return GAMMAVRR_STATUS_SAMPLE_OUT_OF_RANGE;
    }
    return GAMMAVRR_STATUS_INTERNAL_ERROR;
}

} // namespace

extern "C" gammavrr_status gammavrr_process(const uint16_t *input, size_t input_sample_count,
                                            uint16_t *output, size_t output_sample_count,
                                            size_t width, size_t height, const int16_t *lut_r,
                                            const int16_t *lut_g, const int16_t *lut_b,
                                            const gammavrr_params *params) {
    if (input == nullptr || output == nullptr || lut_r == nullptr || lut_g == nullptr ||
        lut_b == nullptr || params == nullptr) {
        return GAMMAVRR_STATUS_INVALID_ARGUMENT;
    }

    try {
        gammavrr::Lut lut{};
        const int16_t *channels[gammavrr::kChannelCount] = {lut_r, lut_g, lut_b};
        for (std::size_t channel = 0; channel < gammavrr::kChannelCount; ++channel) {
            for (std::size_t level = 0; level < gammavrr::kLevelCount; ++level) {
                for (std::size_t node = 0; node < gammavrr::kNodeCount; ++node) {
                    lut[channel][level][node] =
                        channels[channel][level * gammavrr::kNodeCount + node];
                }
            }
        }

        const gammavrr::Config config{
            params->enabled != 0,
            params->zero_setting,
        };
        const gammavrr::Model model(config, std::move(lut));
        const auto error =
            model.process({{input, input_sample_count}, width, height}, params->frame_level,
                          {{output, output_sample_count}, width, height});
        return map_error(error);
    } catch (...) {
        return GAMMAVRR_STATUS_INTERNAL_ERROR;
    }
}

extern "C" const char *gammavrr_status_string(gammavrr_status status) {
    switch (status) {
    case GAMMAVRR_STATUS_OK:
        return "success";
    case GAMMAVRR_STATUS_INVALID_ARGUMENT:
        return "a required pointer argument is null";
    case GAMMAVRR_STATUS_INVALID_FRAME_LEVEL:
        return "frame level must be in the range 0..127";
    case GAMMAVRR_STATUS_INVALID_DIMENSIONS:
        return "image dimensions are invalid or do not match";
    case GAMMAVRR_STATUS_BUFFER_SIZE_MISMATCH:
        return "buffer size does not match width * height * 3";
    case GAMMAVRR_STATUS_SAMPLE_OUT_OF_RANGE:
        return "input contains a sample outside the 12-bit range 0..4095";
    case GAMMAVRR_STATUS_INTERNAL_ERROR:
        return "internal error";
    }
    return "unknown status";
}
