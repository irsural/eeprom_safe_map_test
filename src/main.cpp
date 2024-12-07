#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "eeprom_safe_map.h"
#include "page_mem_demo.h"
#include "safe_map_demo.h"

void make_eeprom(const std::string& eeprom_path, uint32_t page_size, uint32_t pages_count)
{
  std::ofstream ofs(eeprom_path, std::ios::binary | std::ios::out);
  std::vector<char> empty_buf(page_size * pages_count, 0);

  if (!ofs.write(empty_buf.data(), empty_buf.size())) {
    std::cerr << "Ошибка: не удалось создать файл eeprom" << std::endl;
    exit(1);
  }
}

int main()
{
  const std::string eeprom_path = std::string(EEPROM_FILE);
  constexpr uint32_t page_size_bytes = 32;
  constexpr uint32_t sector_size_pages = 16;
  constexpr uint32_t pages_count = 10;

  if (!std::filesystem::exists(eeprom_path)) {
    make_eeprom(eeprom_path, page_size_bytes, pages_count);
  }

  page_mem_demo(eeprom_path, page_size_bytes, pages_count);
  // safe_map_demo(eeprom_path, page_size_bytes, pages_count, sector_size_pages);
}
