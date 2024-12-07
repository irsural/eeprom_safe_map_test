#include "oper_time.h"

oper_time_t::oper_time_t(
  eeprom_safe_map_t<combination_t, uint32_t>* ap_eeprom_safe_map,
  int a_save_time_period_min,
  const combination_t& a_start_combination,
  bool a_enable
) :
  mp_eeprom_safe_map(ap_eeprom_safe_map),
  m_status(status_t::read_time),
  m_is_enabled(a_enable),
  m_combination(a_start_combination),
  m_combination_time(),
  m_least_time(0),
  m_least_time_index(0),
  m_key_index(0),
  m_save_time_period_min(a_save_time_period_min),
  m_delay_timer(irs::make_cnt_s(m_work_timer_period_m))
{
  m_combination_time = m_default_combination_time;
  set_enabled(m_is_enabled);
}

void oper_time_t::set_enabled(bool a_enable)
{
  m_is_enabled = a_enable;
  if (m_is_enabled) {
    if (m_delay_timer.stopped()) {
      m_delay_timer.start();
    }
  } else {
    m_delay_timer.stop();
  }
}

void oper_time_t::tick()
{
  mp_eeprom_safe_map->tick();
  switch (m_status) {
    case status_t::idle: {
      if (m_delay_timer.check()) {
        m_combination_time++;
        m_delay_timer.start();
        if (m_combination_time % m_save_time_period_min == 0) {
          m_status = status_t::write_time;
        }
      }
    } break;
    case status_t::read_time: {
      if (mp_eeprom_safe_map->ready()) {
        if (mp_eeprom_safe_map->get_value(m_combination, m_combination_time)) {
          m_status = status_t::wait_action_end;
        } else {
          m_combination_time = m_default_combination_time;
          m_status = status_t::idle;
        }
      }
    } break;
    case status_t::write_time: {
      if (mp_eeprom_safe_map->ready()) {
        if (mp_eeprom_safe_map->set_value(m_combination, m_combination_time)) {
          m_status = status_t::wait_action_end;
        } else {
          IRS_ASSERT(mp_eeprom_safe_map->get_keys_count() > 1);
          mp_eeprom_safe_map->get_value(mp_eeprom_safe_map->get_key(0), m_tmp_time);
          m_least_time = 0xFFFFFFFF;
          m_least_time_index = 0;
          m_key_index = 0;
          m_status = status_t::find_least_time;
        }
      }
    } break;
    case status_t::find_least_time: {
      if (mp_eeprom_safe_map->ready()) {
        if (m_key_index < mp_eeprom_safe_map->get_keys_count() - 1) {
          if (m_tmp_time < m_least_time) {
            m_least_time = m_tmp_time;
            m_least_time_index = m_key_index;
          }
          m_key_index++;
          mp_eeprom_safe_map->get_value(mp_eeprom_safe_map->get_key(m_key_index), m_tmp_time);
        } else {
          mp_eeprom_safe_map->replace_key(
            mp_eeprom_safe_map->get_key(m_least_time_index), m_combination, m_combination_time
          );
          m_status = status_t::wait_action_end;
        }
      }
    } break;
    case status_t::wait_action_end: {
      if (mp_eeprom_safe_map->ready()) {
        m_status = status_t::idle;
      }
    } break;
  }
}

void oper_time_t::set_combination(const combination_t& a_combination)
{
  if (m_combination != a_combination) {
    m_combination = a_combination;
    m_status = status_t::read_time;
    if (m_is_enabled) {
      m_delay_timer.start();
    }
  }
}

float oper_time_t::get_time() const
{
  return static_cast<float>(
    static_cast<uint32_t>(m_combination_time / 60) +
    static_cast<uint32_t>((m_combination_time % 60) / 6) / 10.
  );
}

void oper_time_t::reset()
{
  m_combination_time = 0;
}

void oper_time_t::reset_all()
{
  reset();
  mp_eeprom_safe_map->reset();
}

bool oper_time_t::ready() const
{
  return m_status == status_t::idle;
}
