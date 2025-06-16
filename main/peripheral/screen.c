#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch.h"
#include "esp_lvgl_port.h"
#include "esp_lcd_touch_xpt2046.h"
#include "esp_check.h"
#include "pin.h"
#include "screen.h"
#include "common.h"

static const char *TAG = "screen";

static esp_lcd_touch_handle_t tp = NULL;
static esp_lcd_panel_io_handle_t tp_io_handle = NULL;
static esp_lcd_panel_io_handle_t io_handle = NULL;
static esp_lcd_panel_handle_t panel_handle = NULL;
static lv_display_t *lvgl_disp = NULL;

static esp_err_t lcd_touch_init(void)
{
    esp_err_t err;
    esp_lcd_panel_io_spi_config_t tp_io_config = ESP_LCD_TOUCH_IO_SPI_XPT2046_CONFIG(LCD_TOUCH_CS);
    err = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &tp_io_config, &tp_io_handle);
    if (err != ESP_OK)
    {
        return err;
    }

    esp_lcd_touch_config_t tp_cfg = {
        .x_max = LCD_H_RES,
        .y_max = LCD_V_RES,
        .rst_gpio_num = -1,
        .int_gpio_num = LCD_TOUCH_IRQ,
        .levels = {
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
    };

    err = esp_lcd_touch_new_spi_xpt2046(tp_io_handle, &tp_cfg, &tp);
    if (err != ESP_OK)
    {
        return err;
    }

    return ESP_OK;
}

static esp_err_t lcd_init(void)
{
    esp_err_t err;

    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << LCD_LED,
    };
    err = gpio_config(&bk_gpio_config);
    if (err != ESP_OK)
    {
        return err;
    }

    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = LCD_DC,
        .cs_gpio_num = LCD_CS,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };

    err = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io_handle);
    if (err != ESP_OK)
    {
        return err;
    }

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_RESET,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB, // LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };
    err = esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle);
    if (err != ESP_OK)
    {
        return err;
    }

    err = gpio_set_level(LCD_LED, LCD_BK_LIGHT_OFF_LEVEL);
    if (err != ESP_OK)
    {
        return err;
    }

    // Reset the display
    err = esp_lcd_panel_reset(panel_handle);
    if (err != ESP_OK)
    {
        return err;
    }

    // Initialize LCD panel
    err = esp_lcd_panel_init(panel_handle);
    if (err != ESP_OK)
    {
        return err;
    }

    // Turn on the screen
    err = esp_lcd_panel_disp_on_off(panel_handle, true);
    if (err != ESP_OK)
    {
        return err;
    }

    // 反色
    // ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));

    // 交换xy轴
    // ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));

    // 镜像
    // ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false));

    // 把buffer中的颜色数据绘制到屏幕上
    // ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, LCD_H_RES, LCD_V_RES, NULL));

    err = gpio_set_level(LCD_LED, LCD_BK_LIGHT_ON_LEVEL);
    if (err != ESP_OK)
    {
        return err;
    }

    return ESP_OK;
}

static esp_err_t lvgl_init(void)
{
    /* Initialize LVGL */
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 4,       /* LVGL task priority */
        .task_stack = 16384,      /* LVGL task stack size */
        .task_affinity = -1,      /* LVGL task pinned to core (-1 is no affinity) */
        .task_max_sleep_ms = 500, /* Maximum sleep in LVGL task */
        .timer_period_ms = 10     /* LVGL timer tick period in ms */
    };
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port initialization failed");

    /* Add LCD screen */
    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = LCD_H_RES * LCD_DRAW_BUFF_HEIGHT,
        .double_buffer = LCD_DRAW_BUFF_DOUBLE,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
        .color_format = LV_COLOR_FORMAT_RGB565,
        .rotation = {
            .swap_xy = true,
            .mirror_x = false,
            .mirror_y = true,
        },
        .flags = {
            .buff_dma = true,
            .swap_bytes = true,
        },
    };
    lvgl_disp = lvgl_port_add_disp(&disp_cfg);

    // 添加触摸
    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = lvgl_disp,
        .handle = tp,
    };
    lvgl_port_add_touch(&touch_cfg);

    return ESP_OK;
}

esp_err_t screen_init(void)
{
    ESP_RETURN_ON_ERROR(spi2_init(), TAG, "spi2_init init failed");
    ESP_RETURN_ON_ERROR(lcd_touch_init(), TAG, "lcd_touch_init init failed");
    ESP_RETURN_ON_ERROR(lcd_init(), TAG, "lcd_init init failed");
    ESP_RETURN_ON_ERROR(lvgl_init(), TAG, "lvgl_init init failed");
    return ESP_OK;
}