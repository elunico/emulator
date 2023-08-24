
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <map>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "bytedefs.hpp"
#include "cpu.hpp"
#include "emulator.hpp"
#include "printer.hpp"
#include "utils.hpp"

int
main(int argc, char const** argv) {
  if (argc < 2) {
    std::cerr << "Provide a program spec file to run" << std::endl;
    return 1;
  }

  // emulator::metaout = emulator::printer::nullprinter;

  emulator::cpu proc;

  std::string name = argv[1];
  auto ext = std::string_view{name.end() - 5, name.end()};
  if (ext == ".prog") {
    emulator::metaout << "Running .prog file" << emulator::endl;
    run_program_file(argv[1], proc);
  } else if (ext == "a.out") {
    emulator::metaout << "Running assembler output" << emulator::endl;
    auto data = load_binary_file(ext);
    proc.set_memory(data.data(), data.size(), 0x0000);
    proc.dump_registers();
    proc.run();
  } else {
    emulator::metaout << "Running .spec container" << emulator::endl;
    run_program_spec(argv[1], proc);
  }
  // run_program_file("jamaica.prog", proc);
}
