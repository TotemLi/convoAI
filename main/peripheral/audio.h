#pragma once

#include "esp_err.h"

// 配置rx对INMP441的采样率为44.1kHz，这是常用的人声采样率
#define SAMPLE_RATE 44100

// #define DMA_FRAME_NUM 1023
#define DMA_FRAME_NUM 1023

// buf size计算方法：根据esp32官方文档，buf size = dma frame num * 声道数 * 数据位宽 / 8
#define BUF_SIZE (DMA_FRAME_NUM * 1 * 32 / 8)
#define WIDTH I2S_DATA_BIT_WIDTH_32BIT
#define MCLK_MULTIPLE I2S_MCLK_MULTIPLE_256

esp_err_t INMP441_init(void);
esp_err_t MAX98357A_init(void);
void play_record(void *args);