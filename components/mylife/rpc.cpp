#include "rpc.h"

#ifdef USE_MYLIFE

#include "esphome/core/application.h"
#include "esphome/core/log.h"
#include "client.h"

namespace esphome {
namespace mylife {

static const char *const TAG = "mylife";

/*

from rpc.ts

topic: base/rpc/services/address
reply: base/rpc/replies/randomId

interface Request {
  input: any; // usually object of params
  replyTopic: string;
}

interface Response {
  output: any;
  error: {
    message: string;
    stacktrace: string;
  };
}
*/

class RestartRpcCall : public RpcCall{
public:
  explicit RestartRpcCall(MylifeClientComponent *client)
   : client_(client) {
  }

  virtual ~RestartRpcCall() = default;

  // Need parse and build as different methods because it uses same buffer
  virtual bool parse_input(JsonVariant& /*input*/) override {
    // fail_safe_ = input["failSafe"];
    return true;
  }

  virtual void execute() override {
    ESP_LOGI(TAG, "Restarting device (using rpc service)...");

    // Let MQTT settle a bit
    App.scheduler.set_timeout(this->client_, "", 100, []() {
      App.safe_reboot();
    });
  }

  virtual void build_output(JsonObject& output) override {
  }

private:
  MylifeClientComponent *client_; // use for set_timeout
};

class RestartRpcService : public RpcService {
public:
  explicit RestartRpcService(MylifeClientComponent *client)
   : client_(client) {
  }

  virtual ~RestartRpcService() = default;

  virtual std::unique_ptr<RpcCall> call() override {
    return make_unique<RestartRpcCall>(this->client_);
  }

private:
  MylifeClientComponent *client_; // use for set_timeout
};


Rpc::Rpc(MylifeClientComponent *client)
 : client_(client) {
}

void Rpc::serve(const std::string &address, std::unique_ptr<RpcService> service) {

  this->services_.push_back(std::move(service));
  auto service_ptr = this->services_.back().get();
  auto topic = this->client_->build_topic({"rpc/services", address});

  this->client_->subscribe(topic, [this, service_ptr](const std::string &topic, const std::string &payload) {
    auto call = service_ptr->call();
    auto call_ptr = call.get();
    std::string reply_topic;

    if (!json::parse_json(payload, [call_ptr, &reply_topic](JsonObject root) -> bool {
      JsonVariant input = root["input"];
      reply_topic = root["replyTopic"].as<std::string>();

      return call_ptr->parse_input(input);
    })) {
      this->reply_error(reply_topic, "Failed to parse input");
      return;
    }

    call_ptr->execute();

    auto reply_payload = json::build_json([call_ptr](JsonObject root) {
      // Note: cannot report error
      auto output = root.createNestedObject("output");
      call_ptr->build_output(output);
    });

    this->client_->publish(reply_topic, reply_payload);
  });
}

void Rpc::reply_error(const std::string &reply_topic, const std::string &message, const std::string &stacktrace) {
    ESP_LOGE(TAG, "RPC error: %s %s", message.c_str(), stacktrace.c_str());

  auto reply_payload = json::build_json([message, stacktrace](JsonObject root) {
    // Note: cannot report error
    auto error = root.createNestedObject("error");
    error["message"] = message;
    error["stacktrace"] = stacktrace;
  });

  this->client_->publish(reply_topic, reply_payload);
}


void Rpc::serve_restart() {
  this->serve("system.restart", make_unique<RestartRpcService>(this->client_));
}

}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE
