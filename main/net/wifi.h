#pragma once

typedef struct
{
    bool wifi_enabled;
    bool wifi_connected;
    uint8_t connect_retry_num; // 重连次数

    // 当前连接的wifi信息
    char *ssid;
    char *password;
    int rssi; // < -40 极强， < -60 强， < -80 中等， 剩余 弱

    // 记住的wifi信息存储在nvs中的
    char *wifi_remember_namespace;
    nvs_handle_t wifi_remeber_handle;
    // 已记住的ssid
    uint8_t remembered_num; // 记住的wifi个数
    char **remembered_ssid; // 记住的wifi名称列表
} wifi_state_t;

esp_err_t wifi_init(void);
esp_err_t enable_wifi(void);