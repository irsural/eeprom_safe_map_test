#include "safe_map_demo.h"

#include <array>
#include <eeprom_safe_map.h>
#include <raw_file_page_mem.h>

void safe_map_demo(
  const std::string& eeprom_path,
  uint32_t page_size_bytes,
  uint32_t pages_count,
  uint32_t sector_size_pages
)
{
  using key_t = std::array<uint8_t, 8>;

  raw_file_page_mem page_mem(eeprom_path, pages_count, page_size_bytes);
  for (uint32_t i = 0; i < pages_count; ++i) {

    eeprom_safe_map_t<key_t, uint32_t> m_eeprom_safe_map(
      &page_mem,
      0,
      pages_count,
      sector_size_pages,
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f}
    );
  }
}

#include "safe_map_demo.h"
