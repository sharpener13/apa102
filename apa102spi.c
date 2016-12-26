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


/*******************************************************************************
 * Function prototypes
 ******************************************************************************/


/*******************************************************************************
 * Private macros
 ******************************************************************************/
#define SPI_DEVICE_SIZE 256


/*******************************************************************************
 * Private variables
 ******************************************************************************/
static char      spi_device[SPI_DEVICE_SIZE + 1];
static uint32_t  spi_speed = 0;
static int       spi_fd    = -1;


/*******************************************************************************
 * Private functions
 ******************************************************************************/

/**
 * SPI Open
 *
 * Expected arguments:
 *     - spi_device: string
 *     - spi_speed : int
 */
int apa102spi_open(char *device, int speed_hz)
{
    uint32_t mode = SPI_CPOL | SPI_CPHA | SPI_NO_CS;
    uint8_t  bits = 8;
    int      ret  = -1;

    strncpy(spi_device, device, SPI_DEVICE_SIZE);
    spi_device[SPI_DEVICE_SIZE] = 0;

    spi_speed = speed_hz;

    spi_fd = open(spi_device, O_WRONLY);
    if (spi_fd < 0)
    {
        fprintf(stderr, "Cannot open SPI device %s\n", spi_device);
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

    ret = ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed);
    if (ret == -1)
    {
        fprintf(stderr, "Cannot setup SPI device speed %2.3f\n", spi_speed / 1000000.0);
        return -4;
    }

    return 0;
}


/**
 * SPI Close
 */
int apa102spi_close(void)
{
    if (spi_fd >= 0)
    {
        close(spi_fd);
        spi_fd = -1;
    }

    return 0;
}


/**
 * SPI Send the whole frame
 *
 * Frame consists of:
 *    - 32 bits LED attention pattern
 *    - 32 x N bits LED data (A, B, G, R), where A is 0b111a_aaaa
 *    - at least N/2 bits of ones - clock latching compensation (each LED delays clock for half of cycle)
 */
int apa102spi_update(uint8_t *data, int length)
{
    struct spi_ioc_transfer tr  = {.delay_usecs = 0,};
    int                     ret = -1;

    if ((data == NULL) || (length <= 0))
        return -1;

    tr.tx_buf = (unsigned long)data;
    tr.len    = length;

    ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);

    if (ret < 1)
    {
        fprintf(stderr, "SPI transfer failed%s\n", (spi_fd < 0) ? " (device probably not open)" : "");
        return -2;
    }

    return 0;
}


/*******************************************************************************
 * End of file
 ******************************************************************************/
