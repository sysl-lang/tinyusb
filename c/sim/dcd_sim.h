#pragma once

#include <stdbool.h>
#include <stdint.h>

// What a test does to the simulated bus. Declared here for dcd_sim.c; the sysl side binds them in
// sh/sysl/tinyusb/c/c.sysl.

void sim_dcd_reset(void);
void sim_dcd_bus_reset(void);
void sim_dcd_setup(const uint8_t setup[8]);

// The length of the transfer queued on an IN endpoint, or -1 when none is.
int32_t sim_dcd_in_pending(uint8_t ep_addr);

// Copies the queued IN transfer out and completes it; the length copied, or -1 when none is queued.
int32_t sim_dcd_in_take(uint8_t ep_addr, uint8_t* out, uint16_t capacity);

// Fills the queued OUT transfer and completes it; the length taken, or -1 when none is queued.
int32_t sim_dcd_out_give(uint8_t ep_addr, const uint8_t* data, uint16_t length);

bool sim_dcd_is_open(uint8_t ep_addr);
bool sim_dcd_is_stalled(uint8_t ep_addr);
uint8_t sim_dcd_address(void);
bool sim_dcd_is_connected(void);
