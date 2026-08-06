/*
 * FatFs R0.16 disk I/O bridge.
 *
 * FatFs calls these C functions. They forward to the active
 * SDCardLowLevel C++ object.
 */
#include "diskio.h"
#include "ff.h"

#ifdef __cplusplus
extern "C" {
#endif

DSTATUS sd_low_level_initialize(void);
DSTATUS sd_low_level_status(void);

DRESULT sd_low_level_read(
    BYTE* buffer,
    LBA_t sector,
    UINT count);

DRESULT sd_low_level_write(
    const BYTE* buffer,
    LBA_t sector,
    UINT count);

DRESULT sd_low_level_ioctl(
    BYTE command,
    void* buffer);

#ifdef __cplusplus
}
#endif

DSTATUS disk_initialize(BYTE pdrv) {
    if (pdrv != 0) {
        return STA_NOINIT;
    }

    return sd_low_level_initialize();
}

DSTATUS disk_status(BYTE pdrv) {
    if (pdrv != 0) {
        return STA_NOINIT;
    }

    return sd_low_level_status();
}

DRESULT disk_read(
    BYTE pdrv,
    BYTE* buffer,
    LBA_t sector,
    UINT count) {

    if (pdrv != 0 ||
        buffer == 0 ||
        count == 0) {
        return RES_PARERR;
    }

    return sd_low_level_read(
        buffer,
        sector,
        count);
}

#if FF_FS_READONLY == 0
DRESULT disk_write(
    BYTE pdrv,
    const BYTE* buffer,
    LBA_t sector,
    UINT count) {

    if (pdrv != 0 ||
        buffer == 0 ||
        count == 0) {
        return RES_PARERR;
    }

    return sd_low_level_write(
        buffer,
        sector,
        count);
}
#endif

DRESULT disk_ioctl(
    BYTE pdrv,
    BYTE command,
    void* buffer) {

    if (pdrv != 0) {
        return RES_PARERR;
    }

    return sd_low_level_ioctl(
        command,
        buffer);
}
