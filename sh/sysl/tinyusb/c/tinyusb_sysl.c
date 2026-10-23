/*
 * What sysl cannot reach in TinyUSB without C: the inline wrappers upstream defines in its headers,
 * the role-and-speed structure `tusb_rhport_init` takes, and the clock TinyUSB asks the application
 * for when it runs without an operating system.
 *
 * Each function is a call or a field and nothing more, so the binding above it decides everything.
 * This file is compiled with TinyUSB's include roots and the configuration the build wrote, so every
 * macro it reads is the one TinyUSB's own objects were built with.
 */
#include "tusb.h"

/* ---- the clock ---- */

static uint32_t (*sysl_clock_read)(void*);
static void* sysl_clock_state;

void sysl_tusb_set_clock(uint32_t (*read)(void*), void* state) {
  sysl_clock_read = read;
  sysl_clock_state = state;
}

#if CFG_TUSB_OS == OPT_OS_NONE
uint32_t tusb_time_millis_api(void) {
  return sysl_clock_read == NULL ? 0 : sysl_clock_read(sysl_clock_state);
}
#endif

/* ---- starting and running the stack ---- */

bool sysl_tusb_init(uint8_t rhport, uint8_t role, uint8_t speed) {
  tusb_rhport_init_t init = { .role = (tusb_role_t) role, .speed = (tusb_speed_t) speed };
  return tusb_rhport_init(rhport, &init);
}

bool sysl_tusb_inited(void) { return tusb_inited(); }

/* One pass over whatever events are waiting, for each role that is compiled in. */
void sysl_tusb_task(void) {
#if CFG_TUD_ENABLED
  if (tud_inited()) tud_task_ext(0, false);
#endif
#if CFG_TUH_ENABLED
  if (tuh_inited()) tuh_task_ext(0, false);
#endif
}

uint8_t sysl_tusb_role_device(void) { return TUSB_ROLE_DEVICE; }
uint8_t sysl_tusb_role_host(void) { return TUSB_ROLE_HOST; }
uint8_t sysl_tusb_speed_auto(void) { return TUSB_SPEED_AUTO; }

/* ---- the device's state, which upstream answers with static inline functions ---- */

#if CFG_TUD_ENABLED
#include <string.h>

/* The configuration descriptor TinyUSB's own descriptor templates (`TUD_CONFIG_DESCRIPTOR`,
 * `TUD_CDC_DESCRIPTOR`, `TUD_MSC_DESCRIPTOR`) spell for the serial ports and the disk this build
 * has, written to `out`; its length. The sysl side builds the same descriptor while compiling, and
 * its tests hold the two to each other. */
uint16_t sysl_tusb_reference_configuration(uint8_t* out, uint16_t bulk_size) {
  const uint8_t ports = CFG_TUD_CDC;
  const bool msc = CFG_TUD_MSC;
  uint16_t total = TUD_CONFIG_DESC_LEN + (uint16_t) (ports * TUD_CDC_DESC_LEN) + (msc ? TUD_MSC_DESC_LEN : 0);
  uint8_t itfs = (uint8_t) (2 * ports + (msc ? 1 : 0));
  uint8_t const head[] = { TUD_CONFIG_DESCRIPTOR(1, itfs, 0, total, 0, 100) };
  uint16_t at = 0;
  memcpy(out + at, head, sizeof(head));
  at += (uint16_t) sizeof(head);
  for (uint8_t p = 0; p < ports; p++) {
    uint8_t const cdc[] = { TUD_CDC_DESCRIPTOR((uint8_t) (2 * p), 0, (uint8_t) (0x81 + 2 * p), 8,
                                               (uint8_t) (0x02 + 2 * p), (uint8_t) (0x82 + 2 * p), bulk_size) };
    memcpy(out + at, cdc, sizeof(cdc));
    at += (uint16_t) sizeof(cdc);
  }
  if (msc) {
    uint8_t const disk[] = { TUD_MSC_DESCRIPTOR((uint8_t) (2 * ports), 0, (uint8_t) (2 * ports + 1),
                                                (uint8_t) (0x81 + 2 * ports), bulk_size) };
    memcpy(out + at, disk, sizeof(disk));
    at += (uint16_t) sizeof(disk);
  }
  return at;
}
#endif

#if CFG_TUD_ENABLED
bool sysl_tud_ready(void) { return tud_ready(); }
bool sysl_tud_mounted(void) { return tud_mounted(); }
bool sysl_tud_suspended(void) { return tud_suspended(); }
bool sysl_tud_connect(void) { return tud_connect(); }
bool sysl_tud_disconnect(void) { return tud_disconnect(); }
bool sysl_tud_remote_wakeup(void) { return tud_remote_wakeup(); }
#endif
