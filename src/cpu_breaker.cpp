#include "cpu_breaker.hpp"

namespace emulator {

cpu_breaker::cpu_breaker(cpu& cpu) : breakee(cpu) {}

u32
cpu_breaker::fetch(const u32& addr) const {
  return breakee.fetch(addr);
}

fetch_result
cpu_breaker::get_next_instruction() {
  return breakee.get_next_instruction();
}

u32&
cpu_breaker::ref_a() {
  return breakee.a;
}

u32&
cpu_breaker::ref_b() {
  return breakee.b;
}

u32&
cpu_breaker::ref_x() {
  return breakee.x;
}

f64&
cpu_breaker::ref_fa() {
  return breakee.fa;
}
f64&
cpu_breaker::ref_fb() {
  return breakee.fb;
}
f64&
cpu_breaker::ref_fx() {
  return breakee.fx;
}

u32&
cpu_breaker::ref_ctrl() {
  return breakee.ctrl;
}

u32&
cpu_breaker::ref_sp() {
  return breakee.sp;
}

u32&
cpu_breaker::ref_ra() {
  return breakee.ra;
}

u32&
cpu_breaker::ref_pc() {
  return breakee.pc;
}

auto&
cpu_breaker::ref_ram() {
  return breakee.ram;
}

u32 const&
cpu_breaker::a() const {
  return breakee.a;
}

u32 const&
cpu_breaker::b() const {
  return breakee.b;
}

u32 const&
cpu_breaker::x() const {
  return breakee.x;
}

f64 const&
cpu_breaker::fa() const {
  return breakee.fa;
}
f64 const&
cpu_breaker::fb() const {
  return breakee.fb;
}
f64 const&
cpu_breaker::fx() const {
  return breakee.fx;
}

u32 const&
cpu_breaker::ctrl() const {
  return breakee.ctrl;
}

u32 const&
cpu_breaker::sp() const {
  return breakee.sp;
}

u32 const&
cpu_breaker::ra() const {
  return breakee.ra;
}

u32 const&
cpu_breaker::pc() const {
  return breakee.pc;
}

auto const&
cpu_breaker::ram() const {
  return breakee.ram;
}

}  // namespace emulator
