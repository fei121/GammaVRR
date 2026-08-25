#ifndef REFRESHTONE_C_API_H
#define REFRESHTONE_C_API_H

#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32) && defined(REFRESHTONE_SHARED)
#if defined(REFRESHTONE_EXPORTS)
#define REFRESHTONE_API __declspec(dllexport)
#else
#define REFRESHTONE_API __declspec(dllimport)
#endif
#elif defined(__GNUC__) && defined(REFRESHTONE_SHARED)
#define REFRESHTONE_API __attribute__((visibility("default")))
#else
#define REFRESHTONE_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum refreshtone_status {
    REFRESHTONE_STATUS_OK = 0,
    REFRESHTONE_STATUS_INVALID_ARGUMENT = 1,
    REFRESHTONE_STATUS_INVALID_FRAME_LEVEL = 2,
    REFRESHTONE_STATUS_INVALID_DIMENSIONS = 3,
    REFRESHTONE_STATUS_BUFFER_SIZE_MISMATCH = 4,
    REFRESHTONE_STATUS_SAMPLE_OUT_OF_RANGE = 5,
    REFRESHTONE_STATUS_INTERNAL_ERROR = 6
};

struct refreshtone_params {
    int enabled;
    int32_t zero_setting;
    int frame_level;
};

/*
 * LUT arrays contain 8 * 256 signed offsets in level-major order.
 * Input and output contain width * height * 3 interleaved RGB samples.
 * In-place processing (input == output) is supported.
 */
REFRESHTONE_API enum refreshtone_status
refreshtone_process(const uint16_t *input, size_t input_sample_count, uint16_t *output,
                 size_t output_sample_count, size_t width, size_t height, const int16_t *lut_r,
                 const int16_t *lut_g, const int16_t *lut_b, const struct refreshtone_params *params);

REFRESHTONE_API const char *refreshtone_status_string(enum refreshtone_status status);

#ifdef __cplusplus
}
#endif

#endif
