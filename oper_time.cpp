#include "oper_time.h"

uint32_t irs::make_cnt_s(uint32_t a_seconds)
{
    return a_seconds * 1000;
}

irs::timer_t::timer_t()
{
}

irs::timer_t::timer_t(uint32_t a_milliseconds)
{
    m_delay = a_milliseconds;
}

bool irs::timer_t::check()
{
    if (m_is_started && !m_is_stopped && !m_is_result_checked && current_time() >= m_time_start_point + m_delay) {
        m_is_result_checked = true;
        return true;
    }
    return false;
}

bool irs::timer_t::stopped() const
{
    if (m_is_stopped) {
        return true;
    }
    return false;
}


void irs::timer_t::start()
{
    if (m_delay != 0) {
        m_time_start_point = current_time() - m_worked_time;
        m_is_started = true;
        m_is_stopped = false;
        m_is_result_checked = false;
    }
}

void irs::timer_t::stop()
{
    if (!m_is_stopped && m_is_started) {
        m_worked_time = current_time() - m_time_start_point;
    }
    m_is_stopped = true;
}

void irs::timer_t::set(uint32_t a_milliseconds)
{
    m_delay = a_milliseconds;
}

uint32_t irs::timer_t::current_time() const
{
    return static_cast<uint32_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}

oper_time_t::oper_time_t(eeprom_safe_map_t<combination_t, uint32_t>* ap_eeprom_safe_map,
  int a_save_time_period_min, const combination_t& a_start_combination, bool a_enable) :
  mp_eeprom_safe_map(ap_eeprom_safe_map),
  m_status(status_t::read_time),
  m_is_enabled(a_enable),
  m_combination(a_start_combination),
  m_combination_time(0),
  m_save_time_period_min(a_save_time_period_min),
  m_delay_timer(irs::make_cnt_s(m_work_timer_period_m/* * 60*/))
{
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
        mp_eeprom_safe_map->get_value(m_combination, m_combination_time);
        m_status = status_t::wait_action_end;
      }
    } break;
    case status_t::write_time: {
      if (mp_eeprom_safe_map->ready()) {
        mp_eeprom_safe_map->set_value(m_combination, m_combination_time);
        m_status = status_t::wait_action_end;
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
  return static_cast<float>(static_cast<uint32_t>(m_combination_time / 60) +
    static_cast<uint32_t>((m_combination_time % 60) / 6) / 10.);
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
