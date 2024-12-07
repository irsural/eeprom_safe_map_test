#ifndef SAFE_MAP_DEMO_H
#define SAFE_MAP_DEMO_H

#include <cstdint>
#include <string>

void safe_map_demo(
  const std::string& eeprom_path,
  uint32_t page_size_bytes,
  uint32_t pages_count,
  uint32_t sector_size_pages
);

#endif //SAFE_MAP_DEMO_H
