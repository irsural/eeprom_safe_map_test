#include "page_mem_demo.h"

#include <cstring>
#include <iostream>

#include "raw_file_page_mem.h"

void wait_page_mem(irs::page_mem_t& page_mem)
{
  // Вызываем тики, пока все данные не запишутся в eeprom
  while (page_mem.status() != irs_st_ready) {
    page_mem.tick();
  }
}

void page_mem_demo(const std::string& eeprom_path, uint32_t page_size_bytes, uint32_t pages_count)
{
  raw_file_page_mem page_mem(eeprom_path, pages_count, page_size_bytes);

  uint8_t buf[page_size_bytes];

  for (uint32_t i = 0; i < pages_count; ++i) {
    // Заполняем каждую страницу eeprom байтом - номером страницы
    memset(buf, static_cast<uint8_t>(i), page_size_bytes);
    page_mem.write_page(buf, i);
    wait_page_mem(page_mem);
  }

  for (uint32_t i = 0; i < pages_count; ++i) {
    // Читаем все страницы
    memset(buf, 0, page_size_bytes);
    page_mem.read_page(buf, i);
    wait_page_mem(page_mem);

    // Выводим считанные данные
    for (uint32_t j = 0; j < page_size_bytes; ++j) {
      std::cout << static_cast<int>(buf[j]) << " ";
    }
    std::cout << std::endl;
  }
}
