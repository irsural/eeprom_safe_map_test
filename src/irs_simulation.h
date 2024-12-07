#ifndef TEST_EEPROM_PC_IRS_SIMULATION_H
#define TEST_EEPROM_PC_IRS_SIMULATION_H

#include <chrono>

using namespace std;
using namespace std::chrono;

#define IRS_ASSERT(pred) \
  if (!(pred)) \
    while (true) \
      ;

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
} // namespace irs

#endif //TEST_EEPROM_PC_IRS_SIMULATION_H
