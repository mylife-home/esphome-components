#pragma once


#include "esphome/core/defines.h"

#ifdef USE_MYLIFE
#ifdef USE_TEXT

#include "esphome/components/text/text.h"
#include "controller.h"

namespace esphome {
namespace mylife {

class MylifeText : public MylifeController {
public:
  explicit MylifeText(MylifeClientComponent *client, text::Text *text);
  virtual ~MylifeText() = default;

  virtual const PluginDefinition *get_plugin_metadata() const override;

protected:
  virtual void publish_states() override;

private:
  void on_set_value(const std::string &value);
  void on_text_change();

  void publish_states_(bool force);

  std::string value_;
  text::Text *text_;
};

}  // namespace mylife
}  // namespace esphome

#endif  // USE_TEXT
#endif  // USE_MYLIFE
