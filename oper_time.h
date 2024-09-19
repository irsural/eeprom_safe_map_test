#ifndef NOISE_GENERATOR_OPER_TIME_H
#define NOISE_GENERATOR_OPER_TIME_H

#include "eeprom_safe_map.h"
#include <array>
#include <sd_page_mem.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

namespace irs {
    uint32_t make_cnt_s(uint32_t a_seconds);

    class timer_t
    {
    public:
        explicit timer_t();
        explicit timer_t(uint32_t a_milliseconds);
        [[nodiscard]] bool check();
        [[nodiscard]] bool stopped() const;
        void start();
        void stop();
        void set(uint32_t a_milliseconds);

    private:
        uint32_t m_time_start_point = 0;
        uint32_t m_delay = 0;
        uint32_t m_worked_time = 0;
        bool m_is_started = false;
        bool m_is_stopped = true;
        bool m_is_result_checked = false;

        [[nodiscard]] uint32_t current_time() const;
    };
}

using combination_t = std::array<uint8_t, 8>;

class oper_time_t
{
public:
  /// \param a_no_save_time_periods Время задержки сохранения в eeprom
  explicit oper_time_t(eeprom_safe_map_t<combination_t, uint32_t>* ap_eeprom_safe_map,
    int a_save_time_period_min, const combination_t& a_start_combination, bool a_enable = false);
  void set_enabled(bool a_enable);
  void tick();
  void set_combination(const combination_t& a_combination);
  [[nodiscard]] float get_time() const;
  void reset();
  void reset_all();
  [[nodiscard]] bool ready() const;

private:
  /// Задержка между инкрементированием таймера в минутах
  static const int m_work_timer_period_m = 1;

  enum class status_t {
    idle,
    read_time,
    write_time,
    wait_action_end,
  };

  eeprom_safe_map_t<combination_t, uint32_t>* mp_eeprom_safe_map;
  status_t m_status;
  bool m_is_enabled;
  combination_t m_combination;
  uint32_t m_combination_time;

  /// Задержка записи в eeprom в периодах инкрементирования
  uint32_t m_save_time_period_min;

  irs::timer_t m_delay_timer;
};

#endif // NOISE_GENERATOR_OPER_TIME_H
