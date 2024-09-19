#ifndef NOISE_GENERATOR_SD_PAGE_MEM_H
#define NOISE_GENERATOR_SD_PAGE_MEM_H

#include <cassert>
#include <cstdint>
#include <iostream>
#include <fstream>

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

  /// \details 0 - без ошибок
  /// \details 1 - выход за пределы разрешенных страниц
  uint8_t m_error_status;
  size_t m_page_count;
  size_t m_page_size;
  size_t m_start_page;

  const string path = "/home/u516/tmp/eeprom.txt";
  fstream m_ios;

  void initialize_io_operation(uint8_t* ap_data, uint32_t a_index, sd_page_mem_state_t a_status);
};

#endif // NOISE_GENERATOR_SD_PAGE_MEM_H
