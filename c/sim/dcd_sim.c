// A device controller with no hardware under it, for the `sim` port.
//
// It implements TinyUSB's device controller interface (device/dcd.h) by recording what the stack asks
// of it: which endpoints it opened, the address it was given, the transfers it started and which
// endpoints it stalled. A test plays the bus: `sim_dcd_setup` delivers a SETUP packet,
// `sim_dcd_in_take` collects the data the stack queued for the host (completing that transfer),
// `sim_dcd_out_give` hands the stack data from the host, and `tud_task` runs between them.

#include "tusb_option.h"

#if CFG_TUD_ENABLED && CFG_TUSB_MCU == OPT_MCU_NONE

#include <string.h>

#include "device/dcd.h"
#include "dcd_sim.h"

typedef struct {
  bool open;
  bool stalled;
  bool pending;
  uint8_t* buffer;
  uint16_t total;
} sim_endpoint_t;

#define SIM_ENDPOINTS 8

static sim_endpoint_t sim_in[SIM_ENDPOINTS];
static sim_endpoint_t sim_out[SIM_ENDPOINTS];
static uint8_t sim_address;
static bool sim_connected;
static bool sim_initialized;
static uint32_t sim_interrupts_enabled;

static sim_endpoint_t* sim_find(uint8_t ep_addr) {
  uint8_t num = tu_edpt_number(ep_addr);
  if (num >= SIM_ENDPOINTS) return NULL;
  return tu_edpt_dir(ep_addr) == TUSB_DIR_IN ? &sim_in[num] : &sim_out[num];
}

void sim_dcd_reset(void) {
  memset(sim_in, 0, sizeof(sim_in));
  memset(sim_out, 0, sizeof(sim_out));
  sim_address = 0;
  sim_connected = false;
  sim_initialized = false;
  sim_interrupts_enabled = 0;
}

bool dcd_init(uint8_t rhport, const tusb_rhport_init_t* rh_init) {
  (void) rhport;
  (void) rh_init;
  sim_dcd_reset();
  sim_initialized = true;
  return true;
}

void dcd_int_enable(uint8_t rhport) { (void) rhport; sim_interrupts_enabled++; }
void dcd_int_disable(uint8_t rhport) { (void) rhport; }
// The controller takes the new address and answers the request's status stage itself, as the
// interface in device/dcd.h says a port must.
void dcd_set_address(uint8_t rhport, uint8_t dev_addr) {
  sim_address = dev_addr;
  dcd_edpt_xfer(rhport, 0x80, NULL, 0, false);
}
void dcd_remote_wakeup(uint8_t rhport) { (void) rhport; }
void dcd_connect(uint8_t rhport) { (void) rhport; sim_connected = true; }
void dcd_disconnect(uint8_t rhport) { (void) rhport; sim_connected = false; }
void dcd_sof_enable(uint8_t rhport, bool en) { (void) rhport; (void) en; }

bool dcd_edpt_open(uint8_t rhport, tusb_desc_endpoint_t const* ep_desc) {
  (void) rhport;
  sim_endpoint_t* ep = sim_find(ep_desc->bEndpointAddress);
  if (ep == NULL) return false;
  ep->open = true;
  ep->stalled = false;
  ep->pending = false;
  return true;
}

void dcd_edpt_close_all(uint8_t rhport) {
  (void) rhport;
  for (int i = 1; i < SIM_ENDPOINTS; i++) {
    memset(&sim_in[i], 0, sizeof(sim_in[i]));
    memset(&sim_out[i], 0, sizeof(sim_out[i]));
  }
}

bool dcd_edpt_xfer(uint8_t rhport, uint8_t ep_addr, uint8_t* buffer, uint16_t total_bytes, bool is_isr) {
  (void) rhport;
  (void) is_isr;
  sim_endpoint_t* ep = sim_find(ep_addr);
  if (ep == NULL) return false;
  ep->pending = true;
  ep->buffer = buffer;
  ep->total = total_bytes;
  return true;
}

void dcd_edpt_stall(uint8_t rhport, uint8_t ep_addr) {
  (void) rhport;
  sim_endpoint_t* ep = sim_find(ep_addr);
  if (ep != NULL) ep->stalled = true;
}

void dcd_edpt_clear_stall(uint8_t rhport, uint8_t ep_addr) {
  (void) rhport;
  sim_endpoint_t* ep = sim_find(ep_addr);
  if (ep != NULL) ep->stalled = false;
}

//--------------------------------------------------------------------+
// What a test does to the bus
//--------------------------------------------------------------------+

void sim_dcd_bus_reset(void) {
  sim_address = 0;
  // The bus runs at the fastest speed the build allows, as a host that can would negotiate.
  dcd_event_bus_reset(0, CFG_TUD_MAX_SPEED == OPT_MODE_HIGH_SPEED ? TUSB_SPEED_HIGH : TUSB_SPEED_FULL, false);
}

void sim_dcd_setup(const uint8_t setup[8]) {
  dcd_event_setup_received(0, setup, false);
}

int32_t sim_dcd_in_pending(uint8_t ep_addr) {
  sim_endpoint_t* ep = sim_find(ep_addr);
  return ep != NULL && ep->pending ? (int32_t) ep->total : -1;
}

int32_t sim_dcd_in_take(uint8_t ep_addr, uint8_t* out, uint16_t capacity) {
  sim_endpoint_t* ep = sim_find(ep_addr);
  if (ep == NULL || !ep->pending || tu_edpt_dir(ep_addr) != TUSB_DIR_IN) return -1;
  uint16_t n = ep->total < capacity ? ep->total : capacity;
  if (n > 0) memcpy(out, ep->buffer, n);
  ep->pending = false;
  dcd_event_xfer_complete(0, ep_addr, ep->total, XFER_RESULT_SUCCESS, false);
  return (int32_t) n;
}

int32_t sim_dcd_out_give(uint8_t ep_addr, const uint8_t* data, uint16_t length) {
  sim_endpoint_t* ep = sim_find(ep_addr);
  if (ep == NULL || !ep->pending || tu_edpt_dir(ep_addr) != TUSB_DIR_OUT) return -1;
  uint16_t n = length < ep->total ? length : ep->total;
  if (n > 0) memcpy(ep->buffer, data, n);
  ep->pending = false;
  dcd_event_xfer_complete(0, ep_addr, n, XFER_RESULT_SUCCESS, false);
  return (int32_t) n;
}

bool sim_dcd_is_open(uint8_t ep_addr) {
  sim_endpoint_t* ep = sim_find(ep_addr);
  return ep != NULL && ep->open;
}

bool sim_dcd_is_stalled(uint8_t ep_addr) {
  sim_endpoint_t* ep = sim_find(ep_addr);
  return ep != NULL && ep->stalled;
}

uint8_t sim_dcd_address(void) { return sim_address; }
bool sim_dcd_is_connected(void) { return sim_connected; }

#endif
