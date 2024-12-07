#include "safe_map_demo.h"

#include <array>
#include <eeprom_safe_map.h>
#include <raw_file_page_mem.h>

using map_key_t = std::array<uint8_t, 8>;

void wait_safe_map(eeprom_safe_map_t<map_key_t, uint32_t>& safe_map)
{
  while (!safe_map.ready()) {
    safe_map.tick();
  }
}

void safe_map_demo(
  const std::string& eeprom_path,
  uint32_t page_size_bytes,
  uint32_t pages_count,
  uint32_t sector_size_pages
)
{
  map_key_t start_key = {1, 2, 3, 4, 5, 6, 7, 8};
  raw_file_page_mem page_mem(eeprom_path, pages_count, page_size_bytes);
  eeprom_safe_map_t<map_key_t, uint32_t> m_eeprom_safe_map(
    &page_mem,
    0,
    pages_count,
    sector_size_pages,
    start_key,
    {0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f}
  );

  m_eeprom_safe_map.reset();
  wait_safe_map(m_eeprom_safe_map);

  // Чтобы лучше понять как работает safe_map можно ставить брейкпоинты на каждый вызов
  // set_value() и смотреть что получилось в hex-редакторе
  // В данном случае запись пройдет "полный круг" по сектору и последнее значение будет записано
  // снова в первую страницу первого сектора
  // В последний байт каждой страницы будет записываться индекс
  for (size_t i = 0; i < sector_size_pages; i++) {
    m_eeprom_safe_map.set_value(start_key, i + 1);
    wait_safe_map(m_eeprom_safe_map);
  }

  // Делаем так, чтобы в каждом секторе был один ключ для демонстрации записи во вторую ячейку
  // первого сектора
  for (size_t i = 0; i < m_eeprom_safe_map.get_data_sectors_count() - 1; i++) {
    map_key_t key;
    key.fill(i);
    m_eeprom_safe_map.set_value(key, 0x10101010 + i);
    wait_safe_map(m_eeprom_safe_map);
  }

  // Этот ключ будет записываться во вторую ячейку первого сектора.
  // Индексы записываются в предпоследнюю ячейку каждой страницы
  map_key_t key;
  key.fill(9);
  for (size_t i = 0; i < sector_size_pages + 1; i++) {
    m_eeprom_safe_map.set_value(key, 0x20202020 + i);
    wait_safe_map(m_eeprom_safe_map);
  }
}
