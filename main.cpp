
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

void
bytes_dump(emulator::byte* start, emulator::byte* end) {
  for (; start < end; start++) {
    std::cout << std::hex << std::noshowbase << (*start < 16 ? "0" : "")
              << static_cast<emulator::u32>(*start);
  }
  std::cout << std::endl;
}

std::vector<emulator::byte>
load_binary_file(std::string_view filename) {
  emulator::metaout << "Attempting to read " << filename << emulator::endl;
  std::ifstream f;
  f.open(filename, std::ios::binary);

  std::vector<emulator::byte> data;
  emulator::i32 buf_size = 4096;
  char buf;
  while (!f.eof()) {
    if (f.bad() || f.fail())
      throw std::runtime_error("Failed to read file load");
    // buf = f.get();
    data.push_back(f.get());
  }
  return data;
}

bool
skip_whitespace(std::ifstream& f) {
  while (!f.eof()) {
    char c = f.get();
    if (!std::isspace(c)) {
      f.unget();  // Put back the non-whitespace character
      return true;
    }
  }
  return false;
}

bool
skip_comment(std::ifstream& f) {
  char c = f.get();
  if (c == '#') {
    // Ignore characters until the end of the line
    while (c != '\n' && !f.eof()) c = f.get();
    if (f.eof()) return false;
  } else {
    f.unget();
  }
  return true;
}

void
run_program_spec(std::string_view config_name, emulator::cpu& oncpu) {
  std::ifstream f;
  f.open(config_name);

  std::map<std::string, emulator::u64> datae{};

  char buf;
  std::vector<char> chars;
  while (!f.eof()) {
    if (f.bad() || f.fail()) throw std::runtime_error("Failed to read file 1");

    char c = f.get();

    if (isspace(c))
      continue;
    else if (c == '#') {
      while ((c = f.get()) != '\n' && !f.eof())
        ;
      f.unget();
    } else if (c == '=') {
      std::vector<char> digits{};
      while (!f.eof()) {
        if (f.bad() || f.fail())
          throw std::runtime_error("Failed to read file 2");
        char c = f.get();
        if (c != '\n' && isspace(c))
          continue;
        else if (c == '#') {
          while ((c = f.get()) != '\n' && !f.eof())
            ;
          f.unget();
        } else if (c == '\n') {
          std::string file = std::string(chars.begin(), chars.end());
          std::string num = std::string(digits.begin(), digits.end());
          emulator::metaout << "Filename is '" << file << "' and num is '"
                            << num << "'" << emulator::endl;
          emulator::u64 n = std::strtoll(num.c_str(), NULL, 16);
          datae[file] = n;
          digits.clear();
          chars.clear();
          goto done;
        } else {
          digits.push_back(c);
        }
      }
    } else {
      chars.push_back(c);
    }
  done:;
  }

  for (auto const& [file, addr] : datae) {
    auto data = load_binary_file(file);
    oncpu.set_memory(data.data(), data.size(), addr);
  }

  return oncpu.run();
}

void
run_program_file(std::string_view filename, emulator::cpu& oncpu) {
  auto data = load_binary_file(filename);

  std::vector<emulator::byte> bootstrap = {{0xE1, 0x00, 0xC0, 0x04}};

  oncpu.set_memory(bootstrap.data(), bootstrap.size(), 0xF000);

  oncpu.set_memory(data.data(), data.size(), 0x3000);
  return oncpu.run();
}

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
