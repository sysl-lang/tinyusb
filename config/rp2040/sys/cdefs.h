// The BSD compiler-attribute macros the pico-sdk's `pico/platform/compiler.h` takes from <sys/cdefs.h>.
//
// A freestanding target has no C library to supply that header, and the SDK's headers are the only
// reader of it here, so the package carries the handful of spellings they use. A target with a libc
// finds the library's own first only if the consumer puts it ahead through `c_include`.

#ifndef SYSL_TINYUSB_SYS_CDEFS_H
#define SYSL_TINYUSB_SYS_CDEFS_H

#ifndef __aligned
#define __aligned(x) __attribute__((__aligned__(x)))
#endif
#ifndef __always_inline
#define __always_inline __attribute__((__always_inline__))
#endif
#ifndef __noinline
#define __noinline __attribute__((__noinline__))
#endif
#ifndef __packed
#define __packed __attribute__((__packed__))
#endif
#ifndef __printflike
#define __printflike(a, b)
#endif
#ifndef __unused
#define __unused __attribute__((__unused__))
#endif
#ifndef __used
#define __used __attribute__((__used__))
#endif
#ifndef __CONCAT1
#define __CONCAT1(a, b) a ## b
#endif
#ifndef __CONCAT
#define __CONCAT(a, b) __CONCAT1(a, b)
#endif
#ifndef __STRING
#define __STRING(a) #a
#endif

#endif
