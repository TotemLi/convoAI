#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_vfs_fat.h"
#include "esp_err.h"
#include "esp_log.h"
#include "common.h"
#include "pin.h"
#include "tf.h"

static sdmmc_card_t *card;
static const char *TAG = "spi";

esp_err_t mount_init(void)
{
    esp_err_t err;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false, // 挂载失败后是否自动格式化
        .max_files = 5,                  // 可同时打开的最大文件数，每打开一个文件需要约1KB内存
        .allocation_unit_size = 16 * 1024,
    };

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI2_HOST; // 使用spi2
    err = spi2_init();
    if (err != ESP_OK)
    {
        return err;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = TF_CS;
    slot_config.host_id = SPI2_HOST;

    err = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);
    return err;
}

esp_err_t unmount_tf(void)
{
    esp_err_t err = esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
    return err;
}

esp_err_t list_dir(const char *path)
{
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        ESP_LOGE(TAG, "open dir: %s err.", path);
        return ESP_FAIL;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        ESP_LOGI(TAG, "name: %s, type: %d, ino: %d", entry->d_name, entry->d_type, entry->d_ino);
    }
    closedir(dir);
    return ESP_OK;
}

esp_err_t make_dir(const char *path)
{
    int ret = mkdir(path, 0777);
    if (ret != 0 && errno != EEXIST)
    {
        ESP_LOGE(TAG, "mkdir path: %s err, ret: %d, errno: %d, errmsg: %s", path, ret, errno, strerror(errno));
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t write_file(const char *path, const char *data)
{
    FILE *f = fopen(path, "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "can't open file: %s, errno: %d", path, errno);
        return ESP_FAIL;
    }

    size_t written_len = fwrite(data, 1, strlen(data), f);
    if (written_len != strlen(data))
    {
        ESP_LOGE(TAG, "write err, write len: %d", written_len);
        return ESP_FAIL;
    }
    fclose(f);
    return ESP_OK;
}