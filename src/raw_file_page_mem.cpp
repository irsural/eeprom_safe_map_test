#include "raw_file_page_mem.h"

#include <cassert>

raw_file_page_mem::raw_file_page_mem(
  const std::string& a_eeprom_filename, size_t a_page_count, size_t a_page_size, size_t a_start_page
) :
  mp_buffer(nullptr),
  m_page_index(0),
  m_status(sd_page_mem_state_t::ready),
  m_page_count(a_page_count),
  m_page_size(a_page_size),
  m_start_page(a_start_page),
  m_eeprom_data(m_page_count)
{
  std::ifstream eeprom_file(a_eeprom_filename, std::ios::binary | std::ios::in);

  for (size_t i = 0; i < m_page_count; ++i) {
    m_eeprom_data[i].resize(m_page_size);

    eeprom_file.get(
      reinterpret_cast<char*>(m_eeprom_data[i].data()),
      static_cast<std::streamsize>(m_page_size + 1)
    );
  }
}

void raw_file_page_mem::read_page(uint8_t* ap_buf, uint32_t a_index)
{
  initialize_io_operation(ap_buf, a_index, sd_page_mem_state_t::read);
}

void raw_file_page_mem::write_page(const uint8_t* ap_buf, uint32_t a_index)
{
  initialize_io_operation(const_cast<uint8_t*>(ap_buf), a_index, sd_page_mem_state_t::write);
}

size_t raw_file_page_mem::page_size() const
{
  return m_page_size;
}

uint32_t raw_file_page_mem::page_count() const
{
  return m_page_count;
}

bool raw_file_page_mem::ready() const
{
  return m_status == sd_page_mem_state_t::ready;
}

irs_status_t raw_file_page_mem::status() const
{
  return m_status == sd_page_mem_state_t::ready ? irs_st_ready : irs_st_busy;
}

void raw_file_page_mem::tick()
{
  switch (m_status) {
    case sd_page_mem_state_t::ready: {
    } break;
    case sd_page_mem_state_t::read: {
      for (int i = 0; i < m_page_size; ++i) {
        mp_buffer[i] = m_eeprom_data[m_page_index][i];
      }
      m_status = sd_page_mem_state_t::ready;
    } break;
    case sd_page_mem_state_t::write: {
      for (int i = 0; i < m_page_size; ++i) {
        m_eeprom_data[m_page_index][i] = mp_buffer[i];
      }
      write_eeprom_file();
      m_status = sd_page_mem_state_t::ready;
    } break;
  }
}

uint8_t raw_file_page_mem::error() const
{
  return 0;
}

uint32_t raw_file_page_mem::start_page() const
{
  return m_start_page;
}

void raw_file_page_mem::initialize_io_operation(
  uint8_t* ap_data, uint32_t a_index, sd_page_mem_state_t a_status
)
{
  assert(a_index < m_page_count);

  mp_buffer = ap_data;
  m_page_index = m_start_page + a_index;
  m_status = a_status;
}

void raw_file_page_mem::write_eeprom_file()
{
  std::ofstream eeprom_file(m_eeprom_filename, std::ios::binary | std::ios::out);
  for (int i = 0; i < m_page_count; ++i) {
    for (int j = 0; j < m_page_size; ++j) {
      eeprom_file << m_eeprom_data[i][j];
    }
  }
}
