#include "printer.hpp"

namespace emulator {
printer const printer::nullprinter = {nullptr};

constexpr const char endl = '\n';

printer metaout = &std::cerr;
printer cpuout = &std::cout;
}  // namespace emulator
