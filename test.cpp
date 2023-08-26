#include <catch2/catch_test_macros.hpp>
#include <cstdint>

#include "bytedefs.hpp"
#include "cpu.hpp"
#include "cpu_breaker.hpp"
#include "memory.hpp"
#include "printer.hpp"
#include "utils.hpp"

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
        emulator::cpu::opcodes::INC_A,   0x0,  0x0,  0x0};
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

TEST_CASE("Memory panics", "[memory-panic]") {
  emulator::memory<uint32_t, 128, 512, uint32_t> mem;
  std::cout << "Working memory" << std::endl;
  REQUIRE_NOTHROW(mem.check_addr(0x10000 - 1));
  REQUIRE_THROWS(mem.check_addr(0x10000));
  REQUIRE_THROWS(mem.check_addr(0x10001));
  REQUIRE_THROWS(mem.check_addr(0x10002));

  REQUIRE(true);
}
