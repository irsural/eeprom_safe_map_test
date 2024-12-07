#ifndef NOISE_GENERATOR_SD_PAGE_MEM_H
#define NOISE_GENERATOR_SD_PAGE_MEM_H

#include <cassert>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "irs_simulation.h"

const uint32_t irs_st_ready = 1;
const uint32_t irs_st_busy = 0;

#define FREE_SPACE_BYTES static_cast<uint32_t>(1 << 11)

using namespace std;

class sd_page_mem_t
{
public:
  explicit sd_page_mem_t(size_t a_page_count, size_t a_page_size, size_t a_start_page = 0);
  typedef size_t size_type;
  void read_page(uint8_t* ap_buf, uint32_t a_index);
  void write_page(const uint8_t* ap_buf, uint32_t a_index);
  [[nodiscard]] size_type page_size() const;
  [[nodiscard]] uint32_t page_count() const;
  [[nodiscard]] bool ready() const;
  [[nodiscard]] uint32_t status() const;
  void tick();
  [[nodiscard]] uint8_t error() const;
  [[nodiscard]] uint32_t start_page() const;

private:
  enum class sd_page_mem_state_t {
    ready,
    write,
    read
  };

  uint8_t* mp_buffer;
  uint32_t m_page_index;
  sd_page_mem_state_t m_status;

  /// \details 0 - Р±РµР· РѕС€РёР±РѕРє
  /// \details 1 - РІС‹С…РѕРґ Р·Р° РїСЂРµРґРµР»С‹ СЂР°Р·СЂРµС€РµРЅРЅС‹С… СЃС‚СЂР°РЅРёС†
  uint8_t m_error_status;
  size_t m_page_count;
  size_t m_page_size;
  size_t m_start_page;

  const string path = "/home/u516/tmp/eeprom.txt";
  fstream m_ios;

  vector<vector<uint8_t>> m_eeprom;

  void initialize_io_operation(uint8_t* ap_data, uint32_t a_index, sd_page_mem_state_t a_status);
  void print_txt();
};

#endif // NOISE_GENERATOR_SD_PAGE_MEM_H
