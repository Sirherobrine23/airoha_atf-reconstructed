#ifndef EN7523_DRAMC_ATF_COMPAT_H
#define EN7523_DRAMC_ATF_COMPAT_H

#include <stdint.h>
#include <common/debug.h>
#include <stdio.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#ifndef readl
#define readl(addr) mmio_read_32((uintptr_t)(addr))
#endif
#ifndef writel
#define writel(val, addr) mmio_write_32((uintptr_t)(addr), (uint32_t)(val))
#endif

/* The recovered vendor sources use the U-Boot name. */
#ifndef __udelay
#define __udelay(usec) udelay(usec)
#endif

#endif
