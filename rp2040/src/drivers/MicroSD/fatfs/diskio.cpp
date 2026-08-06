/*
 * FatFs R0.16 disk I/O bridge.
 *
 * FatFs calls these C functions. They forward to the active
 * SDCardLowLevel C++ object.
 */
#include "ff.h"
#include "diskio.h"
#include "SDCardLowLevel.h"

extern "C"
{

DSTATUS disk_initialize(BYTE pdrv)
{
    if (pdrv != 0)
    {
        return STA_NOINIT;
    }

    return sd_low_level::initialize();
}

DSTATUS disk_status(BYTE pdrv)
{
    if (pdrv != 0)
    {
        return STA_NOINIT;
    }

    return sd_low_level::status();
}

DRESULT disk_read(
    BYTE pdrv,
    BYTE* buffer,
    LBA_t sector,
    UINT count)
{
    if (pdrv != 0 ||
        buffer == nullptr ||
        count == 0)
    {
        return RES_PARERR;
    }

    return sd_low_level::read(
        buffer,
        sector,
        count
    );
}

#if FF_FS_READONLY == 0

DRESULT disk_write(
    BYTE pdrv,
    const BYTE* buffer,
    LBA_t sector,
    UINT count)
{
    if (pdrv != 0 ||
        buffer == nullptr ||
        count == 0)
    {
        return RES_PARERR;
    }

    return sd_low_level::write(
        buffer,
        sector,
        count
    );
}

#endif

DRESULT disk_ioctl(
    BYTE pdrv,
    BYTE command,
    void* buffer)
{
    if (pdrv != 0)
    {
        return RES_PARERR;
    }

    return sd_low_level::ioctl(
        command,
        buffer
    );
}

}