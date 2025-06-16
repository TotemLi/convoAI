#include <stdio.h>
#include "esp_lvgl_port.h"
#include "lvgl.h"

void app_main_display(void)
{

    LV_FONT_DECLARE(fang_song);

    lv_obj_t *scr = lv_scr_act();
    lvgl_port_lock(0);

    // lv_obj_t *img_logo = lv_img_create(scr);
    // lv_img_set_src(img_logo, &esp_logo);
    // lv_obj_align(img_logo, LV_ALIGN_TOP_MID, 0, 20);

    lv_obj_t *label = lv_label_create(scr);
    lv_obj_set_size(label, 100, 80);
    lv_label_set_text(label, "hello totemli");

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &fang_song);
    lv_style_set_text_color(&style, lv_palette_main(LV_PALETTE_RED));

    lv_obj_add_style(label, &style, 0);

    lvgl_port_unlock();
}

// 事件回调函数
void event_handler(lv_event_t *e)
{
    int code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED)
    {
        lv_obj_t *label = lv_event_get_user_data(e);
        lv_label_set_text(label, "itheima");
        printf("clicked\n");
    }
}

void show_button(void)
{
    // 创建按钮
    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    // 创建按钮上的文本
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "button");
    // 给按钮设置点击事件处理
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, label);
}