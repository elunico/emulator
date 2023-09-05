#ifndef UTILS_HPP
#define UTILS_HPP

#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "bytedefs.hpp"
#include "cpu.hpp"
#include "memory.hpp"
#include "printer.hpp"

void bytes_dump(emulator::byte* start, const emulator::byte* end);

std::vector<emulator::byte> load_binary_file(std::string const& filename);

bool skip_whitespace(std::ifstream& f);

bool skip_comment(std::ifstream& f);

std::map<std::string, emulator::u64> parse_program_spec(
    std::string const& config_name);

void run_program_spec(std::string_view config_name, emulator::cpu& oncpu);

void run_program_file(std::string_view filename, emulator::cpu& oncpu);

int parse_int(std::string const& s);

enum class byte_format { hex, dec, oct, bin };

struct memory_print_statement {
  int start, end;
  byte_format format;
};

std::optional<memory_print_statement> parse_print_command(
    std::string const& line);

enum class spacing { off, on };

void print_byte(std::ostream& os,
                emulator::byte b,
                spacing spaced,
                byte_format format = byte_format::hex);

#endif
