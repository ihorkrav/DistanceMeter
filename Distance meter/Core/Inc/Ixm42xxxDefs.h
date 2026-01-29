#ifndef IXM42XXX_DEFS_H
#define IXM42XXX_DEFS_H

#include <stdint.h>

/* -------------------------------------------------
 * Device identification
 * ------------------------------------------------- */
#define IIM_42652_DEVICE_ID     0x6F    /* WHO_AM_I typical value */

/* -------------------------------------------------
 * Register access helpers
 * ------------------------------------------------- */
#define READ_REG    0x80
#define WRITE_REG   0x00

/* Auto-increment for SPI */
#define INC_REG     0x40

/* -------------------------------------------------
 * FIFO / interrupt flags (minimal)
 * ------------------------------------------------- */
#define INT_STATUS        0x2D
#define FIFO_COUNTH       0x2E
#define FIFO_COUNTL       0x2F
#define FIFO_DATA         0x30

#define INT_STATUS_FIFO_FULL   (1 << 0)
#define INT_STATUS_FIFO_THS    (1 << 1)

/* -------------------------------------------------
 * Power & sensor enable
 * ------------------------------------------------- */
#define PWR_MGMT0     0x4E
#define ACCEL_MODE_ON 0x03
#define GYRO_MODE_ON  0x03

/* -------------------------------------------------
 * FIFO config
 * ------------------------------------------------- */
#define FIFO_CONFIG     0x16
#define FIFO_CONFIG1    0x5F

/* -------------------------------------------------
 * Compile-time safety
 * ------------------------------------------------- */
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#endif /* IXM42XXX_DEFS_H */
