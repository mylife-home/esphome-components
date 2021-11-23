#pragma once

#include "esphome/core/defines.h"

#ifdef USE_MYLIFE

#include "esphome/components/time/real_time_clock.h"

namespace esphome {
namespace mylife {

class MylifeClientComponent;

class Logger {
public:
  explicit Logger(MylifeClientComponent *client);

  void set_rtc(time::RealTimeClock *rtc);
private:

  MylifeClientComponent *client_;
  time::RealTimeClock *rtc_{nullptr};
};

}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE
