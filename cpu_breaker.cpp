#include "cpu_breaker.hpp"

namespace emulator {

cpu_breaker::cpu_breaker(cpu& cpu) : breakee(cpu) {}

u32
cpu_breaker::a() const {
  return breakee.a;
}

u32
cpu_breaker::b() const {
  return breakee.b;
}

u32
cpu_breaker::x() const {
  return breakee.x;
}

f64
cpu_breaker::fa() const {
  return breakee.fa;
}
f64
cpu_breaker::fb() const {
  return breakee.fb;
}
f64
cpu_breaker::fx() const {
  return breakee.fx;
}

u32
cpu_breaker::ctrl() const {
  return breakee.ctrl;
}

}  // namespace emulator
