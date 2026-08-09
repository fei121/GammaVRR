#ifndef GAMMAVRR_C_API_H
#define GAMMAVRR_C_API_H

#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32) && defined(GAMMAVRR_SHARED)
#if defined(GAMMAVRR_EXPORTS)
#define GAMMAVRR_API __declspec(dllexport)
#else
#define GAMMAVRR_API __declspec(dllimport)
#endif
#elif defined(__GNUC__) && defined(GAMMAVRR_SHARED)
#define GAMMAVRR_API __attribute__((visibility("default")))
#else
#define GAMMAVRR_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum gammavrr_status {
    GAMMAVRR_STATUS_OK = 0,
    GAMMAVRR_STATUS_INVALID_ARGUMENT = 1,
    GAMMAVRR_STATUS_INVALID_FRAME_LEVEL = 2,
    GAMMAVRR_STATUS_INVALID_DIMENSIONS = 3,
    GAMMAVRR_STATUS_BUFFER_SIZE_MISMATCH = 4,
    GAMMAVRR_STATUS_SAMPLE_OUT_OF_RANGE = 5,
    GAMMAVRR_STATUS_INTERNAL_ERROR = 6
};

struct gammavrr_params {
    int enabled;
    int32_t zero_setting;
    int frame_level;
};

/*
 * LUT arrays contain 8 * 256 signed offsets in level-major order.
 * Input and output contain width * height * 3 interleaved RGB samples.
 * In-place processing (input == output) is supported.
 */
GAMMAVRR_API enum gammavrr_status
gammavrr_process(const uint16_t *input, size_t input_sample_count, uint16_t *output,
                 size_t output_sample_count, size_t width, size_t height, const int16_t *lut_r,
                 const int16_t *lut_g, const int16_t *lut_b, const struct gammavrr_params *params);

GAMMAVRR_API const char *gammavrr_status_string(enum gammavrr_status status);

#ifdef __cplusplus
}
#endif

#endif
