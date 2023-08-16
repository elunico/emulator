
#include <cstdlib>
#include <fstream>
#include <ios>
#include <map>
#include <stdexcept>

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

    if (c == '=') {
      std::vector<char> digits{};
      while (!f.eof()) {
        if (f.bad() || f.fail())
          throw std::runtime_error("Failed to read file 2");
        char c = f.get();
        if (c == '\n') {
          std::string file = std::string(chars.begin(), chars.end());
          std::string num = std::string(digits.begin(), digits.end());
          emulator::metaout << "Filename is " << file << " and num is " << num
                            << emulator::endl;
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

int
main(int argc, char const** argv) {
  if (argc < 2) {
    std::cerr << "Provide a program spec file to run" << std::endl;
    return 1;
  }

  emulator::metaout = emulator::printer::nullprinter;

  emulator::cpu proc;

  run_program_spec(argv[1], proc);
}
