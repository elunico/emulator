
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <map>
#include <stdexcept>
#include <vector>

#include "backwards.hpp"
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
  auto ext = string_section(name, name.size() - 5, name.size());
  if (ext == ".prog") {
    emulator::metaout << "Running .prog file" << emulator::endl;
    run_program_file(argv[1], proc);
  } else if (ext == "a.out") {
    emulator::metaout << "Running assembler output" << emulator::endl;
    auto data = load_binary_file(static_cast<std::string>(ext));
    proc.set_memory(data.data(), data.size(), 0x0000);
    proc.dump_registers();
    proc.run();
  } else if (ext == ".spec") {
    emulator::metaout << "Running .spec container" << emulator::endl;
    run_program_spec(argv[1], proc);
  } else {
    emulator::metaout << "Unknown file type: " << ext
                      << " specify an a.out file a *.spec or a *.prog file"
                      << emulator::endl;
    std::terminate();
  }
  // run_program_file("jamaica.prog", proc);
}
