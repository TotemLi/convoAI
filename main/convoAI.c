#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_err.h"
#include "esp_log.h"
#include "peripheral/tf.h"
#include "peripheral/screen.h"
#include "peripheral/audio.h"
#include "ui/home_page.h"
#include "net/wifi.h"
#include "net/http_server.h"
#include "cjson/cJSON.h"

static const char *TAG = "main";

esp_err_t test_get_handler(httpd_req_t *req)
{

    size_t body_length = req->content_len;
    if (body_length <= 0)
    {
        // TODO 封装一个返回参数错误的函数
        return ESP_OK;
    }

    char *buf = malloc(body_length + 1);
    if (!buf)
    {
        // TODO 封装一个返回服务异常的函数
        return ESP_OK;
    }

    httpd_req_recv(req, buf, body_length);
    buf[body_length] = '\0';

    // ESP_LOGI(TAG, "body: %s", buf);
    cJSON *cjson_body = cJSON_Parse(buf);
    int a = cJSON_GetObjectItem(cjson_body, "a")->valueint;
    ESP_LOGI(TAG, "a: %d", a);
    cJSON *cjson_c = cJSON_GetObjectItem(cjson_body, "c");
    int c_len = cJSON_GetArraySize(cjson_c);
    ESP_LOGI(TAG, "c[%d]: %d", c_len - 1, cJSON_GetArrayItem(cjson_c, c_len - 1)->valueint);

    free(buf);

    cJSON *data = cJSON_CreateObject();
    cJSON_AddNumberToObject(data, "ret", 0);
    cJSON_AddStringToObject(data, "msg", "ok");

    web_resp_json(req, data);
    cJSON_Delete(data);
    return ESP_OK;
}

void app_main(void)
{
    ESP_LOGI(TAG, "start...");

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(wifi_init());
    ESP_ERROR_CHECK(enable_wifi());

    httpd_handle_t server = start_webserver();
    ESP_ERROR_CHECK(webserver_register_handler(server, "/test", HTTP_POST, test_get_handler));

    // // err = mount_init();
    // // if (err != ESP_OK)
    // // {
    // //     ESP_LOGI(TAG, "mount_init err: %d", err);
    // //     return;
    // // }
    // // err = list_dir(MOUNT_POINT "/");
    // // if (err != ESP_OK)
    // // {
    // //     ESP_LOGI(TAG, "list_dir err: %d", err);
    // //     return;
    // // }
    // err = screen_init();
    // if (err != ESP_OK)
    // {
    //     ESP_LOGI(TAG, "mount_init err: %d", err);
    //     return;
    // }
    // app_main_display();
    // // show_button();

    // // err = INMP441_init();
    // // if (err != ESP_OK)
    // // {
    // //     ESP_LOGI(TAG, "INMP441_init err: %d", err);
    // //     return;
    // // }
    // // err = MAX98357A_init();
    // // if (err != ESP_OK)
    // // {
    // //     ESP_LOGI(TAG, "MAX98357A_init err: %d", err);
    // //     return;
    // // }
    // // xTaskCreate(play_record, "play_record", 4096 * 2, NULL, tskIDLE_PRIORITY, NULL);

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
