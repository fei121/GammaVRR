#ifndef REFRESHTONE_MODEL_HPP
#define REFRESHTONE_MODEL_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace refreshtone {

inline constexpr std::size_t kChannelCount = 3;
inline constexpr std::size_t kLevelCount = 8;
inline constexpr std::size_t kNodeCount = 256;
inline constexpr std::uint16_t kMaxSampleValue = 4095;
inline constexpr int kMinFrameLevel = 0;
inline constexpr int kMaxFrameLevel = 127;

using LutChannel = std::array<std::array<std::int16_t, kNodeCount>, kLevelCount>;
using Lut = std::array<LutChannel, kChannelCount>;

struct Config {
    bool enabled{true};
    std::int32_t zero_setting{0};
};

struct ImageView {
    std::span<const std::uint16_t> samples;
    std::size_t width{};
    std::size_t height{};
};

struct MutableImageView {
    std::span<std::uint16_t> samples;
    std::size_t width{};
    std::size_t height{};
};

enum class ProcessError {
    none = 0,
    invalid_frame_level,
    invalid_dimensions,
    input_size_mismatch,
    output_size_mismatch,
    sample_out_of_range,
};

[[nodiscard]] std::string_view to_string(ProcessError error) noexcept;

class Model final {
  public:
    explicit Model(Config config, Lut lut) noexcept;

    [[nodiscard]] ProcessError process(ImageView input, int frame_level,
                                       MutableImageView output) const noexcept;

    [[nodiscard]] const Config &config() const noexcept { return config_; }
    [[nodiscard]] const Lut &lut() const noexcept { return lut_; }

  private:
    Config config_;
    Lut lut_;
};

} // namespace refreshtone

#endif
