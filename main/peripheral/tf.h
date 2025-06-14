#pragma once

#include "esp_err.h"

esp_err_t mount_init(void);
esp_err_t unmount_tf(void);
esp_err_t list_dir(const char *path);
esp_err_t make_dir(const char *path);
esp_err_t write_file(const char *path, const char *data);