# tinyusb

A sysl package for the TinyUSB USB device and host stack, with the controller ports for the
RP2040/RP2350 and the STM32H7 (Synopsys DWC2), and a simulated port for host-side tests.

*An independent sysl package for TinyUSB. It is not affiliated with or endorsed by TinyUSB's authors
or trademark holders.*

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
