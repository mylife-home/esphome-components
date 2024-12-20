#pragma once

#include "command.h"

// Inspired from : https://github.com/loopj/open-rts/blob/main/src/openrts/rts_frame.h

namespace esphome {
namespace somfy_rts {

class Frame {
public:
  static constexpr std::size_t size = 7;

  Frame(Command command, uint16_t rolling_code, uint32_t remote_address);

  void to_bytes(uint8_t *bytes) const;

private:
  /**
    * 8-bit encryption key, typically generated automatically.
    * First 4 bits are typically always 0xA, the last 4 bits increment every
    * button press. Receivers don't typically care about this key, but we'll
    * set it to something sensible automatically when you call a constructor.
    */
  uint8_t encryption_key_ : 8;

  /**
    * The command we are sending, eg "Up", "Down", "My", or combinations of
    * buttons found on remote controls. See rts_command enum.
    */
  Command command_ : 4;

  /**
    * The rolling code of this frame.
    * Rolling codes are sequential, unsigned 16-bit integers sent by remotes
    * that provide some level of protection against cloning attacks.
    * Receivers expect that the rolling code for each paired remote is
    * higher than the previously received code (but not too much higher).
    */
  uint16_t rolling_code_ : 16;

  /**
    * The three-byte remote address sending this frame.
    * This is used by receivers to confirm this is a known, paired remote,
    * and to validate rolling codes.
    */
  uint32_t remote_address_ : 24;  
};

}  // namespace somfy_rts
}  // namespace esphome
