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
bool sysl_tud_ready(void) { return tud_ready(); }
bool sysl_tud_mounted(void) { return tud_mounted(); }
bool sysl_tud_suspended(void) { return tud_suspended(); }
bool sysl_tud_connect(void) { return tud_connect(); }
bool sysl_tud_disconnect(void) { return tud_disconnect(); }
bool sysl_tud_remote_wakeup(void) { return tud_remote_wakeup(); }
#endif
