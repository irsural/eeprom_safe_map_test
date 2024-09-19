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
    m_ios.open(path);
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
        m_ios.seekg(m_page_index * m_page_size);
        m_ios.read((char*)mp_buffer, m_page_size);
      m_status = sd_page_mem_state_t::ready;
    } break;
    case sd_page_mem_state_t::write: {
        m_ios.seekp(m_page_index * m_page_size);
        m_ios.write((char*)mp_buffer, m_page_size);
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
