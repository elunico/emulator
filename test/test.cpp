#include <catch2/catch_test_macros.hpp>
#include <cstdint>

#include "bytedefs.hpp"
#include "cpu.hpp"
#include "cpu_breaker.hpp"
#include "memory.hpp"
#include "printer.hpp"
#include "utils.hpp"

TEST_CASE("byte_of function", "[byte_of]") {
  emulator::u32 number = 0x2244aa66;

  auto b0 = emulator::byte_of<0>(number);
  REQUIRE(b0 == 0x66);

  auto b1 = emulator::byte_of<1>(number);
  REQUIRE(b1 == 0xaa);

  auto b2 = emulator::byte_of<2>(number);
  REQUIRE(b2 == 0x44);

  auto b3 = emulator::byte_of<3>(number);
  REQUIRE(b3 == 0x22);

  emulator::u64 bignum = 0x12'34'56'78'9a'bc'de'fflu;

  auto lb1 = emulator::byte_of<1>(bignum);
  auto lb3 = emulator::byte_of<3>(bignum);
  auto lb5 = emulator::byte_of<5>(bignum);
  auto lb7 = emulator::byte_of<7>(bignum);

  REQUIRE(lb1 == 0xde);
  REQUIRE(lb3 == 0x9a);
  REQUIRE(lb5 == 0x56);
  REQUIRE(lb7 == 0x12);
}

TEST_CASE("Read Program Spec Ignores comments", "[read_program_spec]") {
  emulator::metaout = emulator::printer::nullprinter;

  auto data = parse_program_spec("program-contents/__run.spec");
  REQUIRE(data["program-contents/message.str"] == 0x1000);
  REQUIRE(data["program-contents/square.fn"] == 0x2000);
  REQUIRE(data["program-contents/puts.fn"] == 0x5000);
  REQUIRE(data["program-contents/dist.fn"] == 0x6000);
  REQUIRE(data["program-contents/data-main.fn"] == 0xF000);
}

TEST_CASE("Reading a binary file into a vector", "[load_binary_file]") {
  constexpr const static int arr_size = 32;

  emulator::byte content[arr_size] = {
      0xD2, 0x00, 0x00, 0x01, 0x21, 0x02, 0x01, 0x00, 0xB1, 0x00, 0x00,
      0x02, 0xEA, 0x80, 0x00, 0x0C, 0xCC, 0x00, 0x00, 0x02, 0x61, 0x00,
      0x00, 0x00, 0xE1, 0x00, 0x00, 0x18, 0x30, 0x00, 0x00, 0x00};

  auto bytes = load_binary_file("program-contents/puts.fn");

  REQUIRE(bytes.size() == arr_size);

  for (int i = 0; i < bytes.size(); i++) {
    emulator::metaout << std::hex << (int)bytes[i] << ", ";
    REQUIRE(bytes[i] == content[i]);
  }
}

TEST_CASE("Halting after one tick", "[halt]") {
  emulator::cpu proc;

  emulator::byte data[4] = {0x10, 0, 0, 0};

  proc.set_memory(data, 4, 0xF000);
  proc.tick();
  REQUIRE(proc.is_halted());
}

TEST_CASE("Halting after run", "[halt]") {
  emulator::cpu proc;

  emulator::byte data[4] = {0x10, 0, 0, 0};

  proc.set_memory(data, 4, 0xF000);
  proc.run();
  REQUIRE(proc.is_halted());
}

TEST_CASE("Testing increment instructions", "[increment-instructions]") {
  SECTION("a") {
    emulator::cpu proc;
    emulator::cpu_breaker breaker{proc};

    emulator::byte instructions[] = {
        emulator::cpu::opcodes::LD_IM_A, 0x00, 0x00, 0x40,
        emulator::cpu::opcodes::INC_A,   00,   0x0,  0x0};
    proc.set_memory(instructions, 8, 0xF000);
    proc.tick();
    REQUIRE(breaker.a() == 0x40);
    proc.tick();
    REQUIRE(breaker.a() == 0x41);
  }

  SECTION("b") {
    emulator::cpu proc;
    emulator::cpu_breaker breaker{proc};

    emulator::byte instructions[] = {
        emulator::cpu::opcodes::LD_IM_B, 0x00, 0x00, 0x0,
        emulator::cpu::opcodes::INC_B,   0x0,  0x0,  0x0};
    proc.set_memory(instructions, 8, 0xF000);
    proc.tick();
    REQUIRE(breaker.b() == 0x0);
    proc.tick();
    REQUIRE(breaker.b() == 0x1);
  }

  SECTION("x") {
    emulator::cpu proc;
    emulator::cpu_breaker breaker{proc};

    emulator::byte instructions[] = {
        emulator::cpu::opcodes::LD_IM_X, 0x00, 0x00, 0xff,
        emulator::cpu::opcodes::INC_X,   0x0,  0x0,  0x0};
    proc.set_memory(instructions, 8, 0xF000);
    proc.tick();
    REQUIRE(breaker.x() == 0xff);
    proc.tick();
    REQUIRE(breaker.x() == 0x100);
  }
}

TEST_CASE("Memory in-bounds", "[memory-bounds]") {
  SECTION("normal memory") {
    emulator::memory<uint32_t, 128, 512, uint32_t> mem;

    REQUIRE_NOTHROW(mem.check_addr(0));
    REQUIRE_NOTHROW(mem.check_addr(0x10000 - 1));
  }
}

#if !defined(UNSAFE_READ) && !defined(NO_BOUNDS_CHECK_MEMORY)
TEST_CASE("Memory out-of-bounds", "[memory-bounds]") {
  SECTION("base") {
    emulator::memory<uint32_t, 128, 512, uint32_t> mem;
    REQUIRE_THROWS(mem.check_addr(-1));
    REQUIRE_THROWS(mem.check_addr(0x10000));
    REQUIRE_THROWS(mem.check_addr(0x10001));
    REQUIRE_THROWS(mem.check_addr(0x10002));
  }

  SECTION("empty memory") {
    emulator::memory<uint32_t, 0, 4096> mem;
    REQUIRE_THROWS(mem.check_addr(0));
    REQUIRE_THROWS(mem.check_addr(1));
    REQUIRE_THROWS(mem.check_addr(-1));
  }

  SECTION("empty memory no page size") {
    emulator::memory<uint32_t, 0, 0> mem;
    REQUIRE_THROWS(mem.check_addr(0));
    REQUIRE_THROWS(mem.check_addr(1));
    REQUIRE_THROWS(mem.check_addr(-1));
  }

  SECTION("many pages of no size") {
    emulator::memory<uint32_t, 128, 0> mem;
    REQUIRE_THROWS(mem.check_addr(0));
    REQUIRE_THROWS(mem.check_addr(1));
    REQUIRE_THROWS(mem.check_addr(-1));
  }
}

TEST_CASE("MEMORY ACCESS", "[memory-access]") {
  SECTION("page access big") {
    emulator::page<emulator::u32, 4096> pg;

    for (int i = 0; i < 4096; i++) {
      pg[i] = static_cast<emulator::u8>(i);
    }

    REQUIRE(pg[100] == 100);
    REQUIRE(pg[4095] == static_cast<emulator::u8>(4095));
    REQUIRE_THROWS(pg[4096]);
  }

  SECTION("page access 1") {
    emulator::page<emulator::u32, 1> pg;

    for (int i = 0; i < 1; i++) {
      pg[i] = static_cast<emulator::u8>(i);
    }

    REQUIRE(pg[0] == 0);
    REQUIRE_THROWS(pg[1]);
  }

  SECTION("memory access") {
    emulator::memory<emulator::u32, 2, 4096> pg;

    for (int i = 0; i < 4096 * 2; i++) {
      pg[i] = static_cast<emulator::u8>(i);
    }

    REQUIRE(pg[100] == 100);
    REQUIRE(pg[4095] == static_cast<emulator::u8>(4095));
    REQUIRE_THROWS(pg[4096 * 2]);
  }
}

#endif

TEST_CASE("Register Decoding", "[register-decoding]") {
  SECTION("DSS decode") {
    emulator::cpu proc;
    emulator::cpu_breaker br{proc};

    uint32_t const instruction = 0x00010203;

    auto [d, s, r] = br.register_decode_dss<emulator::u32>(instruction);

    REQUIRE(d == &br.ref_a());
    REQUIRE(s == &br.ref_b());
    REQUIRE(r == &br.ref_x());
  }

  SECTION("DSI decode registers") {
    emulator::cpu proc;
    emulator::cpu_breaker br{proc};

    uint32_t const instruction = 0x000301FF;

    auto [d, s] = br.register_decode_dsi<emulator::u32>(instruction);

    REQUIRE(d == &br.ref_x());
    REQUIRE(s == &br.ref_a());
  }

  SECTION("Both decode registers") {
    emulator::cpu proc;
    emulator::cpu_breaker br{proc};

    uint32_t const instruction = 0x00030102;

    auto [d, s] = br.register_decode_both<emulator::u32>(instruction);

    REQUIRE(d == &br.ref_b());
    REQUIRE(s == &br.ref_a());
  }

  SECTION("First decode registers") {
    emulator::cpu proc;
    emulator::cpu_breaker br{proc};

    uint32_t const instruction = 0x00030101;

    auto r = br.register_decode_first<emulator::u32>(instruction);

    REQUIRE(r == &br.ref_a());
  }
}

TEST_CASE("Jump Offset calculation", "[jumps]") {
  emulator::cpu proc;
  emulator::cpu_breaker br{proc};

  SECTION("offset wrong when instruction passed") {
    uint32_t instruction = 0xE1000025;

    auto i = emulator::get_jump_offset(instruction);
    REQUIRE(i != -0x25);
  }

  SECTION("offset correct from bits forwards short") {
    uint32_t instruction = 0x000025;

    auto i = emulator::get_jump_offset(instruction);
    REQUIRE(i == -0x25);
  }

  SECTION("offset correct from bits forwards long") {
    uint32_t instruction = 0xffffff;

    auto i = emulator::get_jump_offset(instruction);
    REQUIRE(i == 0x7fffff);
  }

  SECTION("offset correct from bits backwards short") {
    uint32_t instruction = 0x000025;

    auto i = emulator::get_jump_offset(instruction);
    REQUIRE(i == -0x25);
  }

  SECTION("offset correct from bits backwards long") {
    uint32_t instruction = 0x7fffff;

    auto i = emulator::get_jump_offset(instruction);
    REQUIRE(i == -0x7fffff);
  }
}

TEST_CASE("fetch from cpu", "[fetch]") {
  emulator::cpu proc;
  emulator::cpu_breaker breaker(proc);

  emulator::byte b[] = {0x01, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8};

  proc.set_memory(b, 8, 0x0000);

  REQUIRE(breaker.fetch(0) == 0x01020304);
  REQUIRE(breaker.fetch(1) == 0x02030405);
  REQUIRE(breaker.fetch(4) == 0x05060708);
}

TEST_CASE("getting instruction", "[get-instructions]") {
  emulator::cpu proc;
  emulator::cpu_breaker breaker(proc);

  emulator::byte b[] = {emulator::cpu::opcodes::LD_IM_A, 0x10, 0x20, 0x30,
                        emulator::cpu::opcodes::XOR_R,   0x01, 0x01, 0x00};

  proc.set_memory(b, 8, 0xF000);

  auto old_pc = breaker.pc();
  auto result = breaker.get_next_instruction();

  REQUIRE(old_pc == (breaker.pc() - 4));

  REQUIRE(result.opcode == emulator::cpu::opcodes::LD_IM_A);
  REQUIRE(result.instruction == 0xA5102030);

  result = breaker.get_next_instruction();

  REQUIRE(old_pc == (breaker.pc() - 8));
  REQUIRE(result.opcode == emulator::cpu::opcodes::XOR_R);
  REQUIRE(result.instruction == 0x05010100);
}

TEST_CASE("getting correct memory address", "[memory-offset-calculation]") {
  SECTION("later") {
    auto const& [page, offset] =
        emulator::memory<emulator::u32, 512, 128>::get_location(405);

    REQUIRE(page == (405 / 128));
    REQUIRE(offset == (405 % 128));
  }

  SECTION("earlier") {
    auto const& [page, offset] =
        emulator::memory<emulator::u32, 128, 512>::get_location(405);

    REQUIRE(page == 0);
    REQUIRE(offset == 405);
  }
}
