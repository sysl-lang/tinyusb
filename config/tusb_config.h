// TinyUSB's configuration, as the package builds it.
//
// Every value comes from the manifest -- the options, the enabled features and the `config` lines of
// the package, its target and its features -- and sysl writes them into `sysl/tinyusb.h` at each
// build. What a controller cannot do is refused here, in words, rather than by the C compiler.

#pragma once

#include "sysl/tinyusb.h"

// A high-speed bulk packet is 512 bytes, and the serial port's stream opens only if its receive buffer
// holds a whole packet; without this the configuration is refused at run time, on the first transfer.
#if CFG_TUD_CDC && CFG_TUD_MAX_SPEED == OPT_MODE_HIGH_SPEED && CFG_TUD_CDC_RX_BUFSIZE < 512
#error "tinyusb: at high speed a serial port's rx_buffer must hold a 512-byte packet; set cdc.rx_buffer to 512 or more"
#endif

#ifdef SYSL_TUSB_RP2
// TinyUSB places its hot functions with the SDK's `__not_in_flash`, which every file of the stack
// then needs to see.
#include "pico.h"
#endif

#if defined(SYSL_TUSB_RP2) && CFG_TUD_ENABLED && CFG_TUH_ENABLED
#error "tinyusb: the RP2040 and RP2350 have one USB controller, which is a device or a host, not both"
#endif
