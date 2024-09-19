#include "sd_page_mem.h"

sd_page_mem_t::sd_page_mem_t(
  size_t a_page_count, size_t a_page_size, size_t a_start_page) :
  mp_buffer(nullptr),
  m_page_index(0),
  m_status(sd_page_mem_state_t::ready),
  m_error_status(0),
  m_page_count(a_page_count),
  m_page_size(a_page_size),
  m_start_page(a_start_page)
{
    m_eeprom.resize(m_page_count);
    m_ios.open(path);
    for (int i = 0; i < m_page_count; ++i) {
        m_eeprom[i].resize(m_page_size);
        m_ios.get((char*)(m_eeprom[i].data()), m_page_size + 1);
    }
    m_ios.close();
}

void sd_page_mem_t::read_page(uint8_t* ap_buf, uint32_t a_index)
{
  initialize_io_operation(ap_buf, a_index, sd_page_mem_state_t::read);
}

void sd_page_mem_t::write_page(const uint8_t* ap_buf, uint32_t a_index)
{
  initialize_io_operation(const_cast<uint8_t*>(ap_buf), a_index, sd_page_mem_state_t::write);
}

size_t sd_page_mem_t::page_size() const
{
  return m_page_size;
}

uint32_t sd_page_mem_t::page_count() const
{
  return m_page_count;
}

bool sd_page_mem_t::ready() const
{
  return m_status == sd_page_mem_state_t::ready;
}

void sd_page_mem_t::tick()
{
  switch (m_status) {
    case sd_page_mem_state_t::ready: {
    } break;
    case sd_page_mem_state_t::read: {
        for (int i = 0; i < m_page_size; ++i) {
            mp_buffer[i] = m_eeprom[m_page_index][i];
        }
        m_status = sd_page_mem_state_t::ready;
    } break;
    case sd_page_mem_state_t::write: {
        for (int i = 0; i < m_page_size; ++i) {
            m_eeprom[m_page_index][i] = mp_buffer[i];
        }
        print_txt();
        m_status = sd_page_mem_state_t::ready;
    } break;
  }
}

uint8_t sd_page_mem_t::error() const
{
  return m_error_status;
}

uint32_t sd_page_mem_t::start_page() const
{
  return m_start_page;
}

void sd_page_mem_t::initialize_io_operation(
  uint8_t* ap_data, uint32_t a_index, sd_page_mem_state_t a_status)
{
  mp_buffer = reinterpret_cast<uint8_t*>(ap_data);
  m_page_index = m_start_page + a_index;
  m_status = a_status;
  /// ѕопытка записи за пределами разрешенных страниц
  if (a_index > m_page_count - 1) {
    m_error_status = 1;
  }
}

void sd_page_mem_t::print_txt()
{
    m_ios.open(path);
    for (int i = 0; i < m_page_count; ++i) {
        for (int j = 0; j < m_page_size; ++j) {
            m_ios << m_eeprom[i][j];
        }
    }
    m_ios.close();
}
