#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "driver/i2s_std.h"
#include "audio.h"
#include "pin.h"

static i2s_chan_handle_t rx_handle;
static i2s_chan_handle_t tx_handle;
uint8_t buf[BUF_SIZE];

esp_err_t INMP441_init(void)
{
    esp_err_t err;
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);

    // dma frame num使用最大值，增大dma一次搬运的数据量，能够提高效率，减小杂音，使用DMA_FRAME_NUM可以做到没有一丝杂音
    chan_cfg.dma_frame_num = DMA_FRAME_NUM;
    err = i2s_new_channel(&chan_cfg, NULL, &rx_handle);
    if (err != ESP_OK)
    {
        return err;
    }

    i2s_std_config_t std_cfg = {
        // .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .clk_cfg = {
            .sample_rate_hz = SAMPLE_RATE,
            .clk_src = I2S_CLK_SRC_DEFAULT,
            .ext_clk_freq_hz = 0,
            .mclk_multiple = MCLK_MULTIPLE,
            // .bclk_div = 8,
        },

        // 虽然inmp441采集数据为24bit，但是仍可使用32bit来接收，中间存储过程不需考虑，只要让声音怎么进来就怎么出去即可
        // .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(WIDTH, I2S_SLOT_MODE_MONO),
        .slot_cfg = {
            .data_bit_width = WIDTH,
            .slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO,
            .slot_mode = I2S_SLOT_MODE_MONO,
            .slot_mask = I2S_STD_SLOT_LEFT,
            .ws_width = WIDTH,
            .ws_pol = false,
            .bit_shift = false,
            .left_align = true,
            .big_endian = false,
            .bit_order_lsb = false,
        },

        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .dout = I2S_GPIO_UNUSED,
            .bclk = INMP441_BCLK,
            .ws = INMP441_WS,
            .din = INMP441_SD,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    err = i2s_channel_init_std_mode(rx_handle, &std_cfg);
    if (err != ESP_OK)
    {
        return err;
    }

    return ESP_OK;
}

esp_err_t MAX98357A_init(void)
{
    esp_err_t err;
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_1, I2S_ROLE_MASTER);
    chan_cfg.dma_frame_num = DMA_FRAME_NUM;
    err = i2s_new_channel(&chan_cfg, &tx_handle, NULL);
    if (err != ESP_OK)
    {
        return err;
    }

    i2s_std_config_t std_cfg = {
        // .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .clk_cfg = {
            .sample_rate_hz = SAMPLE_RATE,
            .clk_src = I2S_CLK_SRC_DEFAULT,
            .ext_clk_freq_hz = 0,
            .mclk_multiple = MCLK_MULTIPLE,
            // .bclk_div = 8,
        },

        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(WIDTH, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .din = I2S_GPIO_UNUSED,
            .bclk = MAX98357A_BCLK,
            .ws = MAX98357A_LR,
            .dout = MAX98357A_DIN,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    err = i2s_channel_init_std_mode(tx_handle, &std_cfg);
    if (err != ESP_OK)
    {
        return err;
    }
    return ESP_OK;
}

void play_record(void *args)
{
    size_t bytes = 0;

    i2s_channel_enable(rx_handle);
    i2s_channel_enable(tx_handle);

    // 一次性读取buf_size数量的音频，即dma最大搬运一次的数据量，读成功后，写入tx，即可通过MAX98357A播放
    while (1)
    {
        if (i2s_channel_read(rx_handle, buf, BUF_SIZE, &bytes, 1000) == ESP_OK)
        {
            i2s_channel_write(tx_handle, buf, BUF_SIZE, &bytes, 1000);
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    vTaskDelete(NULL);
}