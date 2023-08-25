
#ifndef BYTESDEF_H
#define BYTESDEF_H

#include <cinttypes>

namespace emulator {
using i8 = int8_t;
using u8 = uint8_t;
using i16 = int16_t;
using u16 = uint16_t;
using i32 = int32_t;
using u32 = uint32_t;
using i64 = int64_t;
using u64 = uint64_t;

using byte = u8;
using word = u16;

using f64 = double;
using f32 = float;

static_assert(sizeof(f64) == 8);
static_assert(sizeof(f32) == 4);

#ifdef __x86_64__
using f128 = long double;
static_assert(sizeof(f128) == 16);
#endif

}  // namespace emulator

#endif
