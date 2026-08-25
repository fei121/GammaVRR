#include <refreshtone/model.hpp>

#include <cstdint>
#include <vector>

int main() {
    const refreshtone::Model model({}, {});
    const std::vector<std::uint16_t> input{0, 1, 4095};
    std::vector<std::uint16_t> output(3);
    return model.process({input, 1, 1}, 0, {output, 1, 1}) == refreshtone::ProcessError::none ? 0 : 1;
}
