#ifndef PRINTER_HPP
#define PRINTER_HPP
#include <iostream>

namespace emulator {

struct printer {
  static const printer nullprinter;

  std::ostream* stream;

  printer(std::ostream* other) : stream(other) {}
  printer() : stream(nullptr) {}
};

template <typename T>
printer& operator<<(printer& printer, T val) {
  if (printer.stream != nullptr)
    *printer.stream << val;
  return printer;
}

template <>
printer& operator<<(printer& printer, struct printer& (*val)(struct printer&));

printer& endl(printer& os);

extern printer metaout;
extern printer cpuout;

}  // namespace emulator

#endif
