/*************************************************************************//**
 * @file apa102spi.c
 *
 *     APA102 LED chain SPI layer support library.
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/spi/spidev.h>
#include "debug.h"


/*******************************************************************************
 * Private variables
 ******************************************************************************/
static int spi_fd = -1;
static uint32_t spi_speed_hz = 0;


/*******************************************************************************
 * Public functions
 ******************************************************************************/


/*************************************************************************//**
 * Open SPI device
 *
 * root access might be needed.
 *
 * @param[in]    device      SPIdev device name.
 * @param[in]    speed_hz    SPIdev device speed (might be very rough, current
 *                           kernel's module contains bug allowing only limited
 *                           subset of available speeds).
 *
 * @return    zero on success, nonzero otherwise
 *
 ****************************************************************************/
int apa102spi_open(const char *device, uint32_t speed_hz)
{
    uint32_t mode = SPI_CPOL | SPI_CPHA | SPI_NO_CS;
    uint8_t  bits = 8;
    int      ret  = -1;

    spi_fd = open(device, O_WRONLY);
    if (spi_fd < 0)
    {
        fprintf(stderr, "Cannot open SPI device %s\n", device);
        return -1;
    }

    ret = ioctl(spi_fd, SPI_IOC_WR_MODE32, &mode);
    if (ret == -1)
    {
        fprintf(stderr, "Cannot setup SPI device mode %08x\n", mode);
        return -2;
    }

    ret = ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1)
    {
        fprintf(stderr, "Cannot setup SPI device bit count %d\n", bits);
        return -3;
    }

    ret = ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed_hz);
    if (ret == -1)
    {
        fprintf(stderr, "Cannot setup SPI device speed %2.3f\n", speed_hz / 1000000.0);
        return -4;
    }
    spi_speed_hz = speed_hz;

    return 0;
}


/*************************************************************************//**
 * Close SPI device
 *
 * @return    zero on success, nonzero otherwise
 *
 ****************************************************************************/
int apa102spi_close(void)
{
    if (spi_fd >= 0)
    {
        fsync(spi_fd);
        close(spi_fd);
        spi_fd = -1;
    }

    return 0;
}


/*************************************************************************//**
 * Send the whole frame over SPI
 *
 * Frame consists of:
 *    - 32 bits LED attention pattern (all zeros)
 *    - 32 x N bits LED data (A, B, G, R), where A is 0b111a_aaaa, where a is
 *      desired LED brightness (0-31)
 *    - at least N/2 bits of anything - clock latching compensation (each LED
 *      delays clock for half of cycle)
 *
 * @param[in]    data      Frame data
 * @param[in]    length    Frame data length
 *
 * @return    zero on success, nonzero otherwise
 *
 ****************************************************************************/
int apa102spi_update(const uint8_t *data, int length)
{
    struct spi_ioc_transfer tr  = {.delay_usecs = 0, .speed_hz = spi_speed_hz};
    int                     ret = -1;

    if ((data == NULL) || (length <= 0))
    {
        DEBUG_MSG(stderr, "SPI transfer failed (invalid data or length)\n");
        return -1;
    }

    tr.tx_buf = (unsigned long)data;
    tr.len    = length;

    DEBUG_DMP(stdout, data, length, 0, "SPI Data Transfer", NULL);

    ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);

    if (ret < 0)
    {
        DEBUG_FMT(stderr, "SPI transfer failed%s\n", (spi_fd < 0) ? " (device probably not open)" : "");
        return -2;
    }

    return 0;
}


/*******************************************************************************
 * End of file
 ******************************************************************************/
