idf_component_register(SRCS "convoAI.c" "peripheral/common.c" "peripheral/tf.c" "peripheral/screen.c" "peripheral/audio.c" "font/fang_song.c" "ui/home_page.c" "net/wifi.c"
                    INCLUDE_DIRS "." "peripheral" "ui" "net")
                    # REQUIRES esp_lcd lvgl)
