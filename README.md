# tinyusb

A sysl package for the TinyUSB USB device and host stack, with the controller ports for the
RP2040/RP2350 and the STM32H7 (Synopsys DWC2), and a simulated port for host-side tests. It is the
worked example of the `sysl.hocon` manifest (sysl 0.6.0): one vendored tree, left byte for byte as upstream released it,
serves three chips, a device or a host or both, and the classes a program picks.

*An independent sysl package for TinyUSB. It is not affiliated with or endorsed by TinyUSB's authors
or trademark holders.*

```hocon
dependencies {
  tinyusb {
    git      = "github.com/sysl-lang/tinyusb"
    version  = "0.1.0"
    features = [rp2350, cdc]
    options  = { vid = 0x2E8A, pid = 0x000A, product = "Pico 2 serial", cdc.rx_buffer = 512 }
  }
}
```

```sysl
import sh.sysl.tinyusb
import sh.sysl.tinyusb.serial

@export("main")
run() -> int
    tinyusb.set_clock(&board_clock)
    if !tinyusb.start_device(0) then return 1
    val port = serial.Serial(0)
    var chunk: [64]u8 = [0; 64]
    loop
        tinyusb.task()
        if port.connected() && port.available() > 0
            val n = port.read(chunk[..])
            port.write(chunk[0..<n])
            port.flush()
```

## Features

| feature | what it is |
|---|---|
| `rp2040`, `rp2350`, `stm32h7`, `sim` | the `port` choice: exactly one is on. `rp2350` is the RP2040's controller on the RP2350 (Arm or RISC-V); `sim` is a recording controller for tests and exists on hosted machines only |
| `full_speed`, `high_speed` | the `speed` choice (default `full_speed`) |
| `device` (default), `host` | the roles. The RP2 chips have one controller, so both at once is refused with a message; the H7 has two and allows it |
| `cdc`, `msc`, `hid`, `midi` | device classes; each turns `device` on |
| `host_hub`, `host_cdc`, `host_msc`, `host_hid` | host classes; each turns `host` on |

Options: `debug`, `vid`, `pid`, `manufacturer`, `product`, `device.ep0_size`, `host.devices`,
`host.enum_buffer`, `cdc.ports`, `cdc.rx_buffer`, `cdc.tx_buffer`, `msc.buffer`, `hid.interfaces`,
`hid.buffer`, `midi.rx_buffer`, `midi.tx_buffer`, `host_cdc.ports`, `host_hid.interfaces`. Each is a
constant in the package's sysl (`option_cdc_ports`) and a macro in the generated `sysl/tinyusb.h`, which
the package's one `config/tusb_config.h` includes. `sysl build --emit config:tinyusb` prints it.

## What the sysl layer covers

- `sh.sysl.tinyusb`: `set_clock`, `start_device`, `start_host`, `task`, and the device's state.
- `sh.sysl.tinyusb.descriptors`: the device, configuration and string descriptors, built while
  compiling from the options and features (serial ports, then a disk) and exported as TinyUSB's
  `tud_descriptor_*_cb`. A program with its own set gives it to `descriptors.use`.
- `sh.sysl.tinyusb.serial`: a CDC-ACM port.
- `sh.sysl.tinyusb.msc`: a mass-storage disk as a `Disk` trait.
- `sh.sysl.tinyusb.hid`: the report descriptor, `send`. The default descriptors do not declare a HID
  interface, so a program using `hid` brings a configuration descriptor that does.
- `sh.sysl.tinyusb.sim`: the simulated bus, for tests of a program on the host.

`midi` and the host classes compile and link through TinyUSB's own C; they have no sysl API yet.

## What the board supplies

An archive built for a board (`sysl build-c`) leaves symbols for the board's runtime to define. For the
RP2 ports they are the pico-sdk's `irq_add_shared_handler`, `irq_remove_handler`, `irq_set_enabled`,
`critical_section_init`, `panic` and `time_us_64`; for the STM32H7 port, `SystemCoreClock` from ST's
CMSIS system file. The package reads the pico-sdk's headers (`PICO_SDK_PATH`) and ST's CMSIS headers
(`--include-path stm32cube_h7=<STM32CubeH7>`), as `requires` states, and carries the two files the
SDK's CMake generates (`pico/config_autogen.h`, `pico/version.h`, for SDK 2.3.0) and a `sys/cdefs.h` for
a target with no C library. An STM32 application gives its part macro as
`c_defines = { STM32H743xx = true }`.

The H7's DMA mode is off (TinyUSB's default), so nothing needs a DMA-reachable region.

## Vendored upstream

| | |
|---|---|
| project | [hathach/tinyusb](https://github.com/hathach/tinyusb) |
| version | 0.21.0 |
| tarball | `https://github.com/hathach/tinyusb/archive/refs/tags/0.21.0.tar.gz` |
| SHA-256 | `b7cdf35c5ccefb0f61640aff94a732ab4ebcb21498201e4a161545891b23ba01` |
| carried | `src/` and `LICENSE`, byte for byte, in `c/tinyusb/` |
| licence | MIT |

An upgrade is a copy of the new release's `src/` over `c/tinyusb/src`; the manifest's `c` blocks name
what is compiled, so nothing in the vendored tree is edited.

## Tests

```
sysl test .
PICO_SDK_PATH=<pico-sdk> STM32CUBE_H7_PATH=<STM32CubeH7> sysl test .
SYSL_EXTRA_CFLAGS="-fsanitize=address -g" sysl test .
```

The suite plays a host against the `sim` port: it enumerates the device through TinyUSB's own control
pipe and checks the descriptors against the USB specification and against TinyUSB's `TUD_*_DESCRIPTOR`
templates, drives a serial port in both directions through the FIFOs, and runs SCSI commands against a
RAM disk. The `variant:` tests ask the compiler for the suite again under other `--features` and
`--option` settings (two ports, disk only, MIDI, HID, host classes, high speed). The `example:` tests
build the two applications in `examples/` and the RP ports for their boards, and skip when the SDK
trees are not named.
