#include "printer.hpp"

namespace emulator {
printer const printer::nullprinter = {nullptr};

printer&
endl(printer& os) {
  os << "\n";
  if (os.stream != nullptr) os.stream->flush();
  return os;
}

template <>
printer&
operator<<(printer& printer, struct printer& (*val)(struct printer&)) {
  val(printer);
  return printer;
}

printer metaout = &std::cerr;
printer cpuout = &std::cout;
}  // namespace emulator
