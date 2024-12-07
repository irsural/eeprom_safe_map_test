#include "irs_simulation.h"

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