#include "oper_time.h"

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>

using namespace std;
using namespace std::chrono;

const string path = "/home/u516/tmp/eeprom.txt";

ofstream os;

const uint32_t page_size_bytes = 32;
constexpr uint32_t page_count = 32;
const uint32_t sector_size_pages = 16;

array<uint8_t, page_size_bytes> page{};

void init_eeprom_txt()
{
  os.open(path);
  for (int i = 0; i < page_size_bytes * page_count; ++i) {
    os << char(0xff);
  }
  os.close();
}

//#define INIT

int main()
{
#ifdef INIT
  init_eeprom_txt();
#endif
  eeprom_safe_map_t<combination_t, uint32_t> m_eeprom_safe_map(
    new sd_page_mem_t(page_count, page_size_bytes),
    0,
    page_count,
    sector_size_pages,
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f}
  );
  oper_time_t m_oper_time(&m_eeprom_safe_map, 1, {0, 0, 0, 0, 0, 0, 0, 0}, true);
  //    m_oper_time.reset_all();

  irs::timer_t timer(irs::make_cnt_s(5));
  combination_t combination{1, 1, 1, 1, 1, 1, 1, 0};
  bool combination_changed = false;

  uint32_t tmp = 800;

  m_eeprom_safe_map.replace_key({1, 1, 1, 1, 1, 1, 1, 4}, {2, 2, 2, 2, 2, 2, 2, 6}, tmp);

  timer.start();
  while (true) {
    //        if (combination_changed && m_oper_time.ready()) {
    //            m_oper_time.set_combination(combination);
    //            combination_changed = false;
    //        }
    m_oper_time.tick();
    //        if (timer.check()) {
    //            combination[7] = (combination[7] + 1) % 5;
    //            combination_changed = true;
    //            timer.start();
    //        }
  }

  return 0;
}
