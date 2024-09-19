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

const uint32_t eeprom_size_bytes = 1 << 12;
ofstream os;

const uint32_t page_size_bytes = 32;

array<uint8_t, page_size_bytes> page{};

void init_eeprom_txt()
{
    os.open(path);
    for (int i = 0; i < eeprom_size_bytes; ++i) {
        os << "x";
    }
    os.close();
}

int main() {
    irs::timer_t timer(irs::make_cnt_s(2));

    printf("Started\n");
    timer.start();
    while (true) {
        if (timer.check()) {
            printf("One second\n");
        }
    }

//    init_eeprom_txt();

//    for (int i = 0; i < page_size_bytes; ++i) {
//        page[i] = '1';
//    }
//
//    milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
//    printf("%d", ms);


//sd_page_mem_t page_mem(64, page_size_bytes);
//    page_mem.write_page(page.data(), 5);
//    page_mem.tick();


    return 0;
}
