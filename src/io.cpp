#include "gammavrr/io.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace gammavrr {
namespace {

[[nodiscard]] std::string read_ppm_token(std::istream &input, const char *description) {
    while (true) {
        input >> std::ws;
        if (!input.good()) {
            throw std::runtime_error(std::string("missing ") + description);
        }
        if (input.peek() == '#') {
            std::string ignored;
            std::getline(input, ignored);
            continue;
        }
        break;
    }

    std::string token;
    while (input.good()) {
        const auto next = input.peek();
        if (next == std::char_traits<char>::eof() ||
            std::isspace(static_cast<unsigned char>(next)) != 0 || next == '#') {
            break;
        }
        token.push_back(static_cast<char>(input.get()));
    }
    if (token.empty()) {
        throw std::runtime_error(std::string("missing ") + description);
    }
    return token;
}

template <typename T>
[[nodiscard]] T parse_unsigned(std::string_view token, const char *description) {
    unsigned long long parsed = 0;
    const auto *begin = token.data();
    const auto *end = token.data() + token.size();
    const auto result = std::from_chars(begin, end, parsed);
    if (result.ec != std::errc{} || result.ptr != end ||
        parsed > static_cast<unsigned long long>(std::numeric_limits<T>::max())) {
        throw std::runtime_error(std::string("invalid ") + description + ": " + std::string(token));
    }
    return static_cast<T>(parsed);
}

[[nodiscard]] std::size_t checked_sample_count(std::size_t width, std::size_t height) {
    constexpr auto max = std::numeric_limits<std::size_t>::max();
    if (width == 0 || height == 0 || width > max / height || width * height > max / kChannelCount) {
        throw std::runtime_error("invalid or overflowing PPM dimensions");
    }
    return width * height * kChannelCount;
}

[[nodiscard]] LutChannel read_lut_channel(const std::filesystem::path &path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("cannot open LUT file: " + path.string());
    }

    std::array<std::int16_t, kLevelCount * kNodeCount> values{};
    std::size_t count = 0;
    std::string line;
    while (std::getline(input, line)) {
        const auto hash_comment = line.find('#');
        const auto slash_comment = line.find("//");
        const auto comment = std::min(hash_comment, slash_comment);
        if (comment != std::string::npos) {
            line.erase(comment);
        }

        std::istringstream tokens(line);
        long long value = 0;
        while (tokens >> value) {
            if (count == values.size()) {
                throw std::runtime_error("LUT contains more than 2048 values: " + path.string());
            }
            if (value < std::numeric_limits<std::int16_t>::min() ||
                value > std::numeric_limits<std::int16_t>::max()) {
                throw std::runtime_error("LUT value is outside int16 range: " + path.string());
            }
            values[count++] = static_cast<std::int16_t>(value);
        }
        if (!tokens.eof()) {
            throw std::runtime_error("LUT contains a non-integer token: " + path.string());
        }
    }

    if (count != values.size()) {
        throw std::runtime_error("LUT must contain exactly 2048 values, found " +
                                 std::to_string(count) + ": " + path.string());
    }

    LutChannel channel{};
    for (std::size_t level = 0; level < kLevelCount; ++level) {
        for (std::size_t node = 0; node < kNodeCount; ++node) {
            channel[level][node] = values[level * kNodeCount + node];
        }
    }
    return channel;
}

void consume_binary_separator(std::istream &input) {
    const auto separator = input.get();
    if (separator == std::char_traits<char>::eof() ||
        std::isspace(static_cast<unsigned char>(separator)) == 0) {
        throw std::runtime_error("P6 header must end with whitespace");
    }
    if (separator == '\r' && input.peek() == '\n') {
        input.get();
    }
}

} // namespace

Lut read_lut_files(const std::filesystem::path &red_path, const std::filesystem::path &green_path,
                   const std::filesystem::path &blue_path) {
    return {read_lut_channel(red_path), read_lut_channel(green_path), read_lut_channel(blue_path)};
}

Image read_ppm(const std::filesystem::path &path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("cannot open PPM file: " + path.string());
    }

    const auto magic = read_ppm_token(input, "PPM magic");
    PpmFormat format{};
    if (magic == "P3") {
        format = PpmFormat::p3;
    } else if (magic == "P6") {
        format = PpmFormat::p6;
    } else {
        throw std::runtime_error("unsupported PPM format " + magic + "; expected P3 or P6");
    }

    const auto width = parse_unsigned<std::size_t>(read_ppm_token(input, "PPM width"), "PPM width");
    const auto height =
        parse_unsigned<std::size_t>(read_ppm_token(input, "PPM height"), "PPM height");
    const auto max_value = parse_unsigned<std::uint16_t>(read_ppm_token(input, "PPM maximum value"),
                                                         "PPM maximum value");
    if (max_value == 0) {
        throw std::runtime_error("PPM maximum value must be greater than zero");
    }

    Image image{width, height, max_value, format, {}};
    image.samples.resize(checked_sample_count(width, height));

    if (format == PpmFormat::p3) {
        for (auto &sample : image.samples) {
            const auto value =
                parse_unsigned<std::uint16_t>(read_ppm_token(input, "P3 sample"), "P3 sample");
            if (value > max_value) {
                throw std::runtime_error("P3 sample exceeds the declared maximum value");
            }
            sample = value;
        }
    } else {
        consume_binary_separator(input);
        const bool wide = max_value >= 256;
        for (auto &sample : image.samples) {
            const auto high = input.get();
            if (high == std::char_traits<char>::eof()) {
                throw std::runtime_error("P6 pixel data is truncated");
            }
            std::uint16_t value = static_cast<std::uint8_t>(high);
            if (wide) {
                const auto low = input.get();
                if (low == std::char_traits<char>::eof()) {
                    throw std::runtime_error("P6 pixel data is truncated");
                }
                value = static_cast<std::uint16_t>((value << 8) | static_cast<std::uint8_t>(low));
            }
            if (value > max_value) {
                throw std::runtime_error("P6 sample exceeds the declared maximum value");
            }
            sample = value;
        }
    }

    return image;
}

void write_ppm(const Image &image, const std::filesystem::path &path) {
    const auto expected = checked_sample_count(image.width, image.height);
    if (image.samples.size() != expected) {
        throw std::runtime_error("image buffer size does not match width * height * 3");
    }
    if (image.max_value == 0) {
        throw std::runtime_error("PPM maximum value must be greater than zero");
    }
    for (const auto sample : image.samples) {
        if (sample > image.max_value) {
            throw std::runtime_error("image sample exceeds the declared maximum value");
        }
    }

    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("cannot open output PPM file: " + path.string());
    }

    if (image.format == PpmFormat::p3) {
        output << "P3\n" << image.width << ' ' << image.height << '\n' << image.max_value << '\n';
        for (std::size_t index = 0; index < image.samples.size(); index += kChannelCount) {
            output << image.samples[index] << ' ' << image.samples[index + 1] << ' '
                   << image.samples[index + 2] << '\n';
        }
    } else {
        output << "P6\n" << image.width << ' ' << image.height << '\n' << image.max_value << '\n';
        const bool wide = image.max_value >= 256;
        for (const auto sample : image.samples) {
            if (wide) {
                output.put(static_cast<char>((sample >> 8) & 0xFF));
            }
            output.put(static_cast<char>(sample & 0xFF));
        }
    }
    if (!output) {
        throw std::runtime_error("failed while writing output PPM file: " + path.string());
    }
}

} // namespace gammavrr
