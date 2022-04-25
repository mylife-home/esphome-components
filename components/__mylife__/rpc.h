
#pragma once

#include "esphome/core/defines.h"
#include "esphome/components/json/json_util.h"
#include "esphome/components/ota/ota_component.h"
#include <map>
#include <memory>

#ifdef USE_MYLIFE

namespace esphome {
namespace mylife {

class MylifeClientComponent;

class RpcCall {
public:
  virtual ~RpcCall() = default;

  // Need parse and build as different methods because it uses same buffer
  virtual void parse_input(JsonVariant& input) = 0;
  virtual void execute() = 0;
  virtual void build_output(JsonObject& output) = 0;
};

class RpcService {
public:
  virtual ~RpcService() = default;

  virtual std::unique_ptr<RpcCall> call() = 0;
};

template<typename CallType>
class SimpleRpcService : public RpcService {
public:
  virtual ~SimpleRpcService() = default;

  virtual std::unique_ptr<RpcCall> call() override {
    return make_unique<CallType>();
  }
};

class Rpc {
public:
  explicit Rpc(MylifeClientComponent *client);
  void set_ota(ota::OTAComponent *ota);

  void serve(const std::string &address, std::unique_ptr<RpcService> service);

  void serve_restart();
private:

  MylifeClientComponent *client_;
  ota::OTAComponent *ota_;
  // Only as holder
  std::vector<std::unique_ptr<RpcService>> services_;
};

}  // namespace mylife
}  // namespace esphome

#endif  // USE_MYLIFE
