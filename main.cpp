#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdio>
#include <chrono>
#include <array>

#include "oper_time.h"

using namespace std;
using namespace std::chrono;

const string path = "/home/u516/tmp/eeprom.txt";

const uint32_t eeprom_size_bytes = 1 << 13;
ofstream os;

const uint32_t page_size_bytes = 32;

constexpr uint32_t page_count = eeprom_size_bytes / page_size_bytes;

array<uint8_t, page_size_bytes> page{};

void init_eeprom_txt()
{
    os.open(path);
    for (int i = 0; i < eeprom_size_bytes; ++i) {
        os << char(0xff);
    }
    os.close();
}

#define INIT

int main() {
#ifdef INIT
    init_eeprom_txt();
#endif
    eeprom_safe_map_t<combination_t, uint32_t> m_eeprom_safe_map(new sd_page_mem_t(page_count, page_size_bytes), eeprom_size_bytes, 8,
                                                                 { 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0);
    oper_time_t m_oper_time(&m_eeprom_safe_map, 1, { 0, 0, 0, 0, 0, 0, 0, 0 }, true);
    m_oper_time.reset_all();

    irs::timer_t timer(irs::make_cnt_s(3));

    combination_t combination {1, 1, 1, 1, 1, 1, 1, 0};

    timer.start();
    while (true) {
        m_oper_time.tick();
        if (timer.check()) {
            combination[7]++;
            m_oper_time.set_combination(combination);
            timer.start();
        }
    }

    return 0;
}
