#include "utils.hpp"

#include <cstdio>
#include <cstring>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

static std::vector<std::string> formats = {{"hex", "oct", "dec", "bin"}};
static std::unordered_map<std::string, byte_format> format_map = {
    {std::make_pair("hex", byte_format::hex),
     std::make_pair("dec", byte_format::dec),
     std::make_pair("oct", byte_format::oct),
     std::make_pair("bin", byte_format::bin)}};

std::optional<memory_print_statement>
parse_print_command(std::string s) {
  memory_print_statement retval{};

  std::vector<std::string> tokens;

  std::stringstream ss{s};
  std::string tok;

  while (std::getline(ss, tok, ' ')) {
    std::transform(tok.begin(), tok.end(), tok.begin(), tolower);
    tokens.push_back(tok);
  }

  int num_index = 1;
  if (auto p = std::find(formats.begin(), formats.end(), tokens[1]);
      p != formats.end()) {
    retval.format = format_map[*p];
    num_index++;
  } else {
    retval.format = byte_format::hex;
  }

  try {
    retval.start = parse_int(tokens[num_index]);
    retval.end = parse_int(tokens[num_index + 1]);
  } catch (std::invalid_argument e) {
    return std::nullopt;
  }

  return std::make_optional(retval);
}

void
print_byte(std::ostream& os, emulator::byte b, spaced spaced_,
           byte_format format) {
  switch (format) {
    case byte_format::hex:
      os << std::hex;
      break;
    case byte_format::dec:
      os << std::dec;
      break;
    case byte_format::oct:
      os << std::oct;
      break;
    case byte_format::bin:
      std::cerr << "Not currently supported" << std::endl;
      std::terminate();
      break;
  }

  os << std::uppercase << std::noshowbase << (b < 16 ? "0" : "")
     << static_cast<emulator::u32>(b);
  if (spaced_ == spaced::on) {
    os << " ";
  }
}

int
parse_int(std::string const& s) {
  int res;
  if (s.starts_with("0x")) {
    res = std::stoi(s.substr(2), nullptr, 16);
  } else {
    res = std::stoi(s);
  }
  return res;
}

void
bytes_dump(emulator::byte* start, emulator::byte const* end) {
  for (; start < end; start++) {
    std::cout << std::hex << std::noshowbase << (*start < 16 ? "0" : "")
              << static_cast<emulator::u32>(*start);
  }
  std::cout << std::endl;
}

std::vector<emulator::byte>
load_binary_file(std::string const& filename) {
  emulator::metaout << "Attempting to read " << filename << emulator::endl;
  std::ifstream f;
  f.open(filename, std::ios::binary);

  std::vector<emulator::byte> data;
  int buf;
  while (!f.eof()) {
    if (f.bad() || f.fail())
      throw std::runtime_error("Failed to read file load");
    buf = f.get();
    if (buf != EOF) data.push_back(buf);
  }
  return data;
}

bool
skip_whitespace(std::ifstream& f) {
  while (!f.eof()) {
    int c = f.get();
    if (!std::isspace(c)) {
      f.unget();  // Put back the non-whitespace character
      return true;
    }
  }
  return false;
}

bool
skip_comment(std::ifstream& f) {
  int c = f.get();
  if (c == '#') {
    // Ignore characters until the end of the line
    while (c != '\n' && !f.eof()) c = f.get();
    if (f.eof()) return false;
  } else {
    f.unget();
  }
  return true;
}

std::map<std::string, emulator::u64>
parse_program_spec(std::string const& config_name) {
  std::ifstream f;
  f.open(config_name);

  std::map<std::string, emulator::u64> datae{};

  std::vector<char> chars;
  while (!f.eof()) {
    if (f.bad() || f.fail()) throw std::runtime_error("Failed to read file 1");

    int c = f.get();

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
        c = f.get();
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
          emulator::u64 n = std::strtoll(num.c_str(), nullptr, 16);
          datae[file] = n;
          digits.clear();
          chars.clear();
          goto done;
        } else {
          digits.push_back(static_cast<char>(c));
        }
      }
    } else {
      chars.push_back(static_cast<char>(c));
    }
  done:;
  }
  return datae;
}

void
run_program_spec(std::string_view config_name, emulator::cpu& oncpu) {
  auto datae = parse_program_spec(static_cast<std::string>(config_name));

  for (auto const& [file, addr] : datae) {
    auto data = load_binary_file(file);
    oncpu.set_memory(data.data(), data.size(), addr);
  }

  return oncpu.run();
}

void
run_program_file(std::string_view filename, emulator::cpu& oncpu) {
  auto data = load_binary_file(static_cast<std::string>(filename));

  std::vector<emulator::byte> bootstrap = {{0xE1, 0x00, 0xC0, 0x04}};

  oncpu.set_memory(bootstrap.data(), bootstrap.size(), 0xF000);

  oncpu.set_memory(data.data(), data.size(), 0x3000);
  return oncpu.run();
}
