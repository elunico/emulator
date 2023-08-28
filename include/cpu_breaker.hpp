#ifndef CPU_BREAKER_H
#define CPU_BREAKER_H

#include "bytedefs.hpp"
#include "cpu.hpp"

namespace emulator {

struct cpu_breaker {
  cpu& breakee;

  explicit cpu_breaker(cpu& cpu);

  u32&
  ref_a();
  u32&
  ref_b();
  u32&
  ref_x();

  f64&
  ref_fa();
  f64&
  ref_fb();
  f64&
  ref_fx();

  u32&
  ref_ctrl();

  u32&
  ref_sp();

  u32&
  ref_ra();

  u32&
  ref_pc();

  auto&
  ref_ram();

  [[nodiscard]] u32
  a() const;
  [[nodiscard]] u32
  b() const;
  [[nodiscard]] u32
  x() const;

  [[nodiscard]] f64
  fa() const;
  [[nodiscard]] f64
  fb() const;
  [[nodiscard]] f64
  fx() const;

  [[nodiscard]] u32
  ctrl() const;

  [[nodiscard]] u32
  sp() const;

  [[nodiscard]] u32
  ra() const;

  [[nodiscard]] u32
  pc() const;

  [[nodiscard]] auto
  ram() const;

  template <typename RegType>
  [[nodiscard]] std::tuple<RegType*, RegType*, RegType*>
  register_decode_dss(u32 instruction) const {
    return breakee.register_decode_dss<RegType>(instruction);
  }

  template <typename RegType>
  [[nodiscard]] std::pair<RegType*, RegType*>
  register_decode_dsi(u32 instruction) const {
    return breakee.register_decode_dsi<RegType>(instruction);
  }

  template <typename RegType>
  [[nodiscard]] std::pair<RegType*, RegType*>
  register_decode_both(u32 instruction) const {
    return breakee.register_decode_both<RegType>(instruction);
  }

  template <typename RegType>
  [[nodiscard]] RegType*
  register_decode_first(u32 instruction) const {
    return breakee.register_decode_first<RegType>(instruction);
  }

  template <typename RegType>
  [[nodiscard]] RegType*
  reg_get_by_index(u32 reg_index) const {
    return breakee.reg_get_by_index<RegType>(reg_index);
  }
};
}  // namespace emulator
#endif
