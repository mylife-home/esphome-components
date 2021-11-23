#include "logger.h"

#ifdef USE_MYLIFE

#include "client.h"
#include "esphome/core/application.h"
#ifdef USE_LOGGER
#include "esphome/components/logger/logger.h"
#endif
#include <queue>

namespace esphome {
namespace mylife {

static const char *const TAG = "mylife";
static constexpr std::size_t max_buffer_size = 4096;

enum class TimestampType : uint8_t { time, uptime };

struct Timestamp {
  TimestampType type;

  union {
    time_t time;
    uint32_t uptime;
  };
};

struct LogItem {
  std::size_t allocated_size;
  Timestamp timestamp;
  int level;
  std::string tag;
  std::string message;
};

class LogBuffer {
public:
  void add(Timestamp &&timestamp, int level, const char *tag, const char *message) {
    auto item = make_unique<LogItem>();

    item->timestamp = std::move(timestamp);
    item->level = level;
    item->tag = tag;
    item->message = message;

    item->allocated_size = sizeof(LogItem) + item->tag.size() + item->message.size(); // Not very accurate but at last give some info

    while (this->size_ + item->allocated_size > max_buffer_size && this->buffer_.size() > 0) {
      this->buffer_.pop();
    }

    this->size_ += item->allocated_size;
    this->buffer_.emplace(std::move(item));
  }

  void flush(const std::function<bool(const Timestamp& timestamp, int level, const char *tag, const char *message)> &callback) {
    while (this->buffer_.size() > 0) {
      const auto &item = this->buffer_.front();

      if (!callback(item->timestamp , item->level, item->tag.c_str(), item->message.c_str())) {
        break;
      }

      this->size_ -= item->allocated_size;
      this->buffer_.pop();
    }
  }

private:
  std::queue<std::unique_ptr<LogItem>> buffer_;
  std::size_t size_;
};

Logger::Logger(MylifeClientComponent *client)
 : client_(client)
 , buffer_(make_unique<LogBuffer>()) {

  client_->add_on_online_callback([this](bool online) {
    if (online) {
      this->try_flush();
    }
  });

#ifdef USE_LOGGER
  if (logger::global_logger != nullptr) {
    logger::global_logger->add_on_log_callback([this](int level, const char *tag, const char *message) {
      auto ts = this->now();

      if (!this->output(ts, level, tag, message)) {
        this->buffer_->add(std::move(ts), level, tag, message);
      }
    });
  }
#endif
}

Logger::~Logger() = default;

void Logger::set_rtc(time::RealTimeClock *rtc) {
  this->rtc_ = rtc;

  this->rtc_->add_on_time_sync_callback([this]() {
    this->rtc_synced_ = true;
    this->try_flush();
  });
}

bool Logger::can_output() const {
  return this->rtc_synced_ && this->client_->is_connected();
}

bool Logger::output(const Timestamp& timestamp, int level, const char *tag, const char *message) {
  if (!this->can_output()) {
    return false;
  }

  auto topic = App.get_name() + "/logger";

  auto generator = [&](JsonObject &root) {
    root["name"] = TAG;
    root["instanceName"] = App.get_name();
    root["hostname"] = App.get_name();
    root["pid"] = 0;
    root["level"] = this->to_level(level);
    root["msg"] = "test message";
    root["time"] = this->to_time(timestamp);
    root["v"] = 0;
  };

  size_t len;
  const char *payload = json::build_json(generator, &len);

  return client_->publish(topic, payload, len);
}

void Logger::try_flush() {
  if (this->can_output()) {
    buffer_->flush([&](const Timestamp& timestamp, int level, const char *tag, const char *message) -> bool {
      return this->output(timestamp, level, tag, message);
    });
  }
}

Timestamp Logger::now() {
  Timestamp ts;

  if (this->rtc_synced_) {
    ts.type = TimestampType::time;
    ts.time = this->rtc_->timestamp_now();
  } else {
    ts.type = TimestampType::uptime;
    ts.uptime = millis();
  }

  return ts;
}

std::string Logger::to_time(const Timestamp &timestamp) {
  time_t time{0};

  switch(timestamp.type) {
    case TimestampType::time: {
      time = timestamp.time;
      break;
    }

    case TimestampType::uptime: {
      auto diff = (millis() - timestamp.uptime) / 1000;
      time = this->rtc_->timestamp_now() - diff;
      break;
    }
  }

  // 2021-11-23T15:01:04.655Z
  return time::ESPTime::from_epoch_local(time).strftime("%FT%TZ");
}

int Logger::to_level(int level) {
  /* from bunyan doc:
  "fatal" (60): The service/app is going to stop or become unusable now. An operator should definitely look into this soon.
  "error" (50): Fatal for a particular request, but the service/app continues servicing other requests. An operator should look at this soon(ish).
  "warn" (40): A note on something that should probably be looked at by an operator eventually.
  "info" (30): Detail on regular operation.
  "debug" (20): Anything else, i.e. too verbose to be included in "info" level.
  "trace" (10): Logging from external libraries used by your app or very detailed application logging.
  */

  switch(level) {
  case ESPHOME_LOG_LEVEL_ERROR:
    return 50;

  case ESPHOME_LOG_LEVEL_WARN:
    return 40;

  case ESPHOME_LOG_LEVEL_INFO:
    return 30;

  case ESPHOME_LOG_LEVEL_CONFIG:
  case ESPHOME_LOG_LEVEL_DEBUG:
    return 20;

  case ESPHOME_LOG_LEVEL_VERBOSE:
  case ESPHOME_LOG_LEVEL_VERY_VERBOSE:
    return 10;

  default:
    return 0;
  }
}


}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE
