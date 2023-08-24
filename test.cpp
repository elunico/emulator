#include <catch2/catch_test_macros.hpp>

#include "bytedefs.hpp"
#include "cpu.hpp"
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

TEST_CASE("Reading a binary file into a vector", "[read_binary_file]") {
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
