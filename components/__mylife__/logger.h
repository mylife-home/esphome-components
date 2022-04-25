#pragma once

#include "esphome/core/defines.h"

#ifdef USE_MYLIFE

#include "esphome/components/time/real_time_clock.h"
#include <memory>

namespace esphome {
namespace mylife {

class MylifeClientComponent;
class LogBuffer;
struct Timestamp;

class Logger {
public:
  explicit Logger(MylifeClientComponent *client);
  ~Logger();

  void set_rtc(time::RealTimeClock *rtc);
private:
  bool can_output() const;
  bool output(const Timestamp& timestamp, int level, const char *tag, const char *message);
  void try_flush();
  Timestamp now();
  std::string to_time(const Timestamp &timestamp);
  int to_level(int level);
  std::string to_message(const char *msg);

  MylifeClientComponent *client_;
  time::RealTimeClock *rtc_{nullptr};
  std::unique_ptr<LogBuffer> buffer_;
  bool rtc_synced_{false};
};

}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE
