#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "peripheral/tf.h"
#include "peripheral/screen.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "start...");
    esp_err_t err;
    err = mount_init();
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "mount_init err: %d", err);
        return;
    }
    // err = list_dir(MOUNT_POINT "/");
    // if (err != ESP_OK)
    // {
    //     ESP_LOGI(TAG, "list_dir err: %d", err);
    //     return;
    // }
    err = screen_init();
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "mount_init err: %d", err);
        return;
    }
    // app_main_display();
    show_button();

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
