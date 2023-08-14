#include <iostream>

namespace emulator {

struct printer {
  std::ostream* stream;

  printer(std::ostream* other) : stream(other) {}
  printer() : stream(nullptr) {}
};

template <typename T>
printer&
operator<<(printer& printer, T val) {
  if (printer.stream != nullptr) *printer.stream << val;
  return printer;
}

constexpr const char endl = '\n';

}  // namespace emulator
