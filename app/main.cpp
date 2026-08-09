#include "gammavrr/io.hpp"
#include "gammavrr/model.hpp"

#include <CLI/CLI.hpp>

#include <cstdint>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <utility>

int main(int argc, char **argv) {
    CLI::App app{"Apply the bit-exact GammaVRR offset model to a 12-bit RGB PPM image"};
    app.set_version_flag("--version", GAMMAVRR_VERSION);

    std::filesystem::path input_path;
    std::filesystem::path output_path;
    std::filesystem::path lut_r_path;
    std::filesystem::path lut_g_path;
    std::filesystem::path lut_b_path;
    int frame_level = 0;
    std::int32_t zero_setting = 0;
    bool disabled = false;
    std::string output_format;

    app.add_option("-i,--input", input_path, "Input 12-bit PPM image")->required();
    app.add_option("-o,--output", output_path, "Output PPM image")->required();
    app.add_option("--lut-r", lut_r_path, "Red 8x256 offset LUT")->required();
    app.add_option("--lut-g", lut_g_path, "Green 8x256 offset LUT")->required();
    app.add_option("--lut-b", lut_b_path, "Blue 8x256 offset LUT")->required();
    app.add_option("-f,--frame-level", frame_level, "Frame level in the range 0..127")
        ->required()
        ->check(CLI::Range(gammavrr::kMinFrameLevel, gammavrr::kMaxFrameLevel));
    app.add_option("--zero-setting", zero_setting, "Offset subtracted after interpolation");
    app.add_flag("--disable", disabled, "Bypass compensation and copy the input");
    app.add_option("--output-format", output_format, "Override output format: p3 or p6")
        ->check(CLI::IsMember({"p3", "p6"}));

    CLI11_PARSE(app, argc, argv);

    try {
        auto input = gammavrr::read_ppm(input_path);
        if (input.max_value != gammavrr::kMaxSampleValue) {
            std::cerr << "error: the model requires a PPM maximum value of 4095, found "
                      << input.max_value << '\n';
            return 2;
        }

        auto lut = gammavrr::read_lut_files(lut_r_path, lut_g_path, lut_b_path);
        const gammavrr::Model model({!disabled, zero_setting}, std::move(lut));

        auto output = input;
        if (output_format == "p3") {
            output.format = gammavrr::PpmFormat::p3;
        } else if (output_format == "p6") {
            output.format = gammavrr::PpmFormat::p6;
        }

        const auto error = model.process({input.samples, input.width, input.height}, frame_level,
                                         {output.samples, output.width, output.height});
        if (error != gammavrr::ProcessError::none) {
            std::cerr << "error: " << gammavrr::to_string(error) << '\n';
            return 2;
        }

        gammavrr::write_ppm(output, output_path);
        std::cout << "processed " << input.width << 'x' << input.height
                  << " RGB image at frame level " << frame_level << " -> " << output_path.string()
                  << '\n';
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
