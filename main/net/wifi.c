#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_log.h"

static EventGroupHandle_t s_wifi_event_group;
static const char *TAG = "wifi";
static int s_retry_num = 0;
static nvs_handle_t wifi_remeber_handle;
static uint8_t remembered_num = 0;
static char **remembered_ssid; // 已存储的wifi账号

// 把连接成功的wifi记住
static char *remeber_ssid, *remeber_password;

void wifi_connect(char *ssid, char *password, wifi_auth_mode_t authmode)
{
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = authmode,
        },
    };

    strcpy((char *)wifi_config.sta.ssid, ssid);
    strcpy((char *)wifi_config.sta.password, password);
    remeber_ssid = ssid;
    remeber_password = password;

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
        s_retry_num = 0;
        // 存储账号密码
        esp_err_t err = nvs_set_str(wifi_remeber_handle, remeber_ssid, remeber_password);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "save ssid err: %s", esp_err_to_name(err));
        }
        else
        {
            ESP_LOGI(TAG, "save ssid success, ssid: %s, password: %s", remeber_ssid, remeber_password);
        }
    }
}

void get_all_wifi_remeber(void)
{
    uint8_t alloc_num = 4;
    remembered_ssid = (char **)malloc(4 * sizeof(char *));
    nvs_iterator_t it = NULL;
    esp_err_t err = nvs_entry_find(NVS_DEFAULT_PART_NAME, "wifi_remember", NVS_TYPE_STR, &it);
    while (err == ESP_OK)
    {
        if (remembered_num >= alloc_num)
        {
            // 空间不够用了，再分配一点
            char **temp = (char **)realloc(remembered_ssid, (alloc_num + 4) * sizeof(char *));
            if (temp == NULL)
            {
                printf("内存分配失败\n");
            }
            remembered_ssid = temp;
            alloc_num += 4;
        }
        nvs_entry_info_t info;
        nvs_entry_info(it, &info);
        ESP_LOGI(TAG, "remebered ssid: %s", info.key);
        err = nvs_entry_next(&it);
        remembered_ssid[remembered_num] = info.key;
        remembered_num++;
    }
    nvs_release_iterator(it);
}

void wifi_init(void)
{

    ESP_ERROR_CHECK(nvs_open("wifi_remember", NVS_READWRITE, &wifi_remeber_handle));
    get_all_wifi_remeber();

    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}