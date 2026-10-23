// TinyUSB's configuration, as the package builds it.
//
// Every value comes from the manifest -- the options, the enabled features and the `config` lines of
// the package, its target and its features -- and sysl writes them into `sysl/tinyusb.h` at each
// build. What a controller cannot do is refused here, in words, rather than by the C compiler.

#pragma once

#include "sysl/tinyusb.h"

#if defined(SYSL_TUSB_RP2) && CFG_TUD_ENABLED && CFG_TUH_ENABLED
#error "tinyusb: the RP2040 and RP2350 have one USB controller, which is a device or a host, not both"
#endif
