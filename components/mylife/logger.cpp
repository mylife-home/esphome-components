#include "logger.h"

#ifdef USE_MYLIFE

#include "client.h"
#ifdef USE_LOGGER
#include "esphome/components/logger/logger.h"
#endif

namespace esphome {
namespace mylife {

static const char *const TAG = "mylife";

Logger::Logger(MylifeClientComponent *client)
 : client_(client) {

  client_->add_on_online_callback([this](bool online) {
    if (online) {
      // TODO
    }
  });


#ifdef USE_LOGGER
  if (logger::global_logger != nullptr) {
    logger::global_logger->add_on_log_callback([this](int level, const char *tag, const char *message) {
      // TODO
    });
  }
#endif
}

void Logger::set_rtc(time::RealTimeClock *rtc) {
  this->rtc_ = rtc;

  // TODO: on sync
}

}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE
