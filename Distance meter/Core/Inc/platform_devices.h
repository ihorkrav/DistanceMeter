#ifndef PLATFORM_DEVICES_H
#define PLATFORM_DEVICES_H

#include "stm32g4xx.h"
#include <stdint.h>
#include <stdlib.h>

/* -------------------------------------------------
 *  Time abstraction
 * ------------------------------------------------- */
static inline void inv_delay_ms(uint32_t ms)
{
    /* You already use SysTick elsewhere */
    while (ms--) {
        for (volatile uint32_t i = 0; i < (SystemCoreClock / 8000); i++) {
            __NOP();
        }
    }
}

/* Some InvenSense headers expect this macro */
#define INV_DELAY_MS(ms) inv_delay_ms(ms)

/* -------------------------------------------------
 *  Memory abstraction
 * ------------------------------------------------- */
#define inv_malloc   malloc
#define inv_free     free

/* -------------------------------------------------
 *  Bus abstraction
 * ------------------------------------------------- */
typedef enum {
    INV_BUS_SPI = 0,
    INV_BUS_I2C = 1
} inv_bus_type_t;

/* -------------------------------------------------
 *  Generic device descriptor
 * ------------------------------------------------- */
typedef struct {
    inv_bus_type_t bus_type;
    void *bus_handle;        /* unused (SPI handled manually) */
    uint8_t i2c_addr;        /* unused */
} inv_device_t;

/* -------------------------------------------------
 *  Assert compatibility
 * ------------------------------------------------- */
#ifndef INV_ASSERT
#define INV_ASSERT(x) do { if (!(x)) while(1); } while(0)
#endif

#endif /* PLATFORM_DEVICES_H */
