#ifndef CPU_BREAKER_H
#define CPU_BREAKER_H

#include "bytedefs.hpp"
#include "cpu.hpp"

namespace emulator {

struct cpu_breaker {
  cpu& breakee;

  explicit cpu_breaker(cpu& cpu);

  u32
  a() const;
  u32
  b() const;
  u32
  x() const;

  f64
  fa() const;
  f64
  fb() const;
  f64
  fx() const;

  u32
  ctrl() const;

  u32
  sp() const;

  u32
  ra() const;

  u32
  pc() const;

  auto
  ram() const;
};
}  // namespace emulator
#endif
