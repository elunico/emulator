#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#include "bytedefs.hpp"
#include "cpu.hpp"
#include "memory.hpp"
#include "printer.hpp"

void
bytes_dump(emulator::byte* start, emulator::byte* end);

std::vector<emulator::byte>
load_binary_file(std::string_view filename);

bool
skip_whitespace(std::ifstream& f);

bool
skip_comment(std::ifstream& f);

std::map<std::string, emulator::u64>
parse_program_spec(std::string_view config_name);

void
run_program_spec(std::string_view config_name, emulator::cpu& oncpu);

void
run_program_file(std::string_view filename, emulator::cpu& oncpu);
