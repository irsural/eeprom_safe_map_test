#ifndef NOISE_GENERATOR_SD_PAGE_MEM_H
#define NOISE_GENERATOR_SD_PAGE_MEM_H

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

enum irs_status_t {
  irs_st_busy,
  irs_st_ready,
  irs_st_error
};

namespace irs {
class page_mem_t
{
public:
  typedef size_t size_type;
  virtual ~page_mem_t() {};
  virtual void read_page(uint8_t* ap_buf, unsigned int a_index) = 0;
  virtual void write_page(const uint8_t* ap_buf, unsigned int a_index) = 0;
  virtual size_type page_size() const = 0;
  virtual unsigned int page_count() const = 0;
  virtual irs_status_t status() const = 0;
  virtual void tick() = 0;
};
} // namespace irs

class raw_file_page_mem : public irs::page_mem_t
{
public:
  explicit raw_file_page_mem(
    const std::string& a_eeprom_filename,
    size_t a_page_count,
    size_t a_page_size,
    size_t a_start_page = 0
  );
  typedef size_t size_type;
  void read_page(uint8_t* ap_buf, uint32_t a_index);
  void write_page(const uint8_t* ap_buf, uint32_t a_index);
  [[nodiscard]] size_type page_size() const;
  [[nodiscard]] uint32_t page_count() const;
  [[nodiscard]] bool ready() const;
  [[nodiscard]] irs_status_t status() const;
  void tick();
  [[nodiscard]] uint8_t error() const;
  [[nodiscard]] uint32_t start_page() const;

private:
  enum class status_t {
    ready,
    write,
    read
  };

  const std::string m_eeprom_filename;
  const size_t m_page_count;
  const size_t m_page_size;
  const size_t m_start_page;

  uint8_t* mp_buffer;
  uint32_t m_page_index;
  status_t m_status;

  uint32_t m_current_byte;

  std::vector<std::vector<uint8_t>> m_eeprom_data;

  void initialize_io_operation(uint8_t* ap_data, uint32_t a_index, status_t a_status);
  void write_eeprom_file();
};

#endif // NOISE_GENERATOR_SD_PAGE_MEM_H
