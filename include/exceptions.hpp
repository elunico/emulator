#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>

namespace emulator {
class no_such_opcode : public std::invalid_argument {
 public:
  explicit no_such_opcode(std::string const& msg)
      : std::invalid_argument(msg) {}
};

class no_such_register : public std::invalid_argument {
 public:
  explicit no_such_register(std::string const& msg)
      : std::invalid_argument(msg) {}
};
}  // namespace emulator

#endif
