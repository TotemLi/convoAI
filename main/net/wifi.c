#include <string.h>
#include <errno.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "wifi.h"

static wifi_state_t wifi_state;

static EventGroupHandle_t s_wifi_event_group;
static const char *TAG = "wifi";

void wifi_connect(char *ssid, char *password, wifi_auth_mode_t authmode)
{
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = authmode,
        },
    };

    strcpy((char *)wifi_config.sta.ssid, ssid);
    strcpy((char *)wifi_config.sta.password, password);
    wifi_state.ssid = ssid;
    wifi_state.password = password;

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    esp_wifi_connect();
}

static char *auth_mode_str(int authmode)
{
    switch (authmode)
    {
    case WIFI_AUTH_OPEN:
        return "tWIFI_AUTH_OPEN";
    case WIFI_AUTH_OWE:
        return "WIFI_AUTH_OWE";
    case WIFI_AUTH_WEP:
        return "tWIFI_AUTH_WEP";
    case WIFI_AUTH_WPA_PSK:
        return "WIFI_AUTH_WPA_PSK";
    case WIFI_AUTH_WPA2_PSK:
        return "WIFI_AUTH_WPA2_PSK";
    case WIFI_AUTH_WPA_WPA2_PSK:
        return "WIFI_AUTH_WPA_WPA2_PSK";
    case WIFI_AUTH_ENTERPRISE:
        return "WIFI_AUTH_ENTERPRISE";
    case WIFI_AUTH_WPA3_PSK:
        return "WIFI_AUTH_WPA3_PSK";
    case WIFI_AUTH_WPA2_WPA3_PSK:
        return "WIFI_AUTH_WPA2_WPA3_PSK";
    case WIFI_AUTH_WPA3_ENTERPRISE:
        return "WIFI_AUTH_WPA3_ENTERPRISE";
    case WIFI_AUTH_WPA2_WPA3_ENTERPRISE:
        return "WIFI_AUTH_WPA2_WPA3_ENTERPRISE";
    case WIFI_AUTH_WPA3_ENT_192:
        return "WIFI_AUTH_WPA3_ENT_192";
    default:
        return "WIFI_AUTH_UNKNOWN";
    }
}

void wifi_scan_result(uint16_t scan_num)
{
    uint16_t number = scan_num;
    wifi_ap_record_t *ap_info = (wifi_ap_record_t *)malloc(scan_num * sizeof(wifi_ap_record_t));
    memset(ap_info, 0, scan_num * sizeof(wifi_ap_record_t)); // 将内存置空

    uint16_t ap_count = 0;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_LOGI(TAG, "Total APs scanned = %u, actual AP number ap_info holds = %u", ap_count, number);
    for (int i = 0; i < number; i++)
    {
        ESP_LOGI(TAG, "ssid: %s, rssi: %d, authmode: %s, channel: %d", ap_info[i].ssid, ap_info[i].rssi, auth_mode_str(ap_info[i].authmode), ap_info[i].primary);
    }

    // 判断是否有认识的wifi，有则选择一个进行自动连接

    // 连接wifi
    wifi_connect("XIAOJING", "xiaojing", WIFI_AUTH_WPA2_PSK);
    free(ap_info);
}

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        // wifi启动事件
        // 进行wifi扫描
        esp_wifi_scan_start(NULL, false);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        // wifi断开事件
        printf("get event WIFI_EVENT_STA_DISCONNECTED");
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE)
    {
        // wifi扫描结束，打印结果
        wifi_scan_result(10);
    }

    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        // 获取到ip事件
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        // 存储账号密码
        esp_err_t err = nvs_set_str(wifi_state.wifi_remeber_handle, wifi_state.ssid, wifi_state.password);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "save ssid err: %s", esp_err_to_name(err));
        }
        else
        {
            ESP_LOGI(TAG, "save ssid success, ssid: %s", wifi_state.ssid);
        }
    }
}

esp_err_t load_remembered_ssid(void)
{
    ESP_ERROR_CHECK(nvs_open(wifi_state.wifi_remember_namespace, NVS_READWRITE, &wifi_state.wifi_remeber_handle));
    uint8_t alloc_num = 4;
    wifi_state.remembered_ssid = (char **)malloc(4 * sizeof(char *));
    nvs_iterator_t it = NULL;
    esp_err_t err = nvs_entry_find(NVS_DEFAULT_PART_NAME, wifi_state.wifi_remember_namespace, NVS_TYPE_STR, &it);
    while (err == ESP_OK)
    {
        if (wifi_state.remembered_num >= alloc_num)
        {
            // 空间不够用了，再分配一点
            char **temp = (char **)realloc(wifi_state.remembered_ssid, (alloc_num + 4) * sizeof(char *));
            if (temp == NULL)
            {
                return (esp_err_t)errno;
            }
            wifi_state.remembered_ssid = temp;
            alloc_num += 4;
        }
        nvs_entry_info_t info;
        nvs_entry_info(it, &info);
        ESP_LOGI(TAG, "remebered ssid: %s", info.key);
        err = nvs_entry_next(&it);
        wifi_state.remembered_ssid[wifi_state.remembered_num] = info.key;
        wifi_state.remembered_num++;
    }
    nvs_release_iterator(it);
    return ESP_OK;
}

esp_err_t wifi_init(void)
{
    wifi_state.wifi_enabled = false;
    wifi_state.wifi_connected = false;
    wifi_state.connect_retry_num = 3;
    wifi_state.wifi_remember_namespace = "wifi_remember";

    ESP_ERROR_CHECK(load_remembered_ssid());
    return ESP_OK;
}

esp_err_t enable_wifi(void)
{
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    return ESP_OK;
}

esp_err_t close_wifi(void)
{
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());
    return ESP_OK;
}