#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/spi_master.h"
#include "pin.h"

#define SPI2_TRANSFER_MAX_SIZE 51200

esp_err_t spi2_init(void)
{
    // TODO 这儿需要加锁，不然会出现资源竞争问题
    static bool initialized = false;
    if (initialized)
    {
        return ESP_OK;
    }
    spi_bus_config_t buscfg = {
        .sclk_io_num = SPI2_SCK,
        .mosi_io_num = SPI2_MOSI,
        .miso_io_num = SPI2_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = SPI2_TRANSFER_MAX_SIZE,
    };
    esp_err_t err = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (err != ESP_OK)
    {
        return err;
    }
    initialized = true;
    return ESP_OK;
}