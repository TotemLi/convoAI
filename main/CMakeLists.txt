set(SRC_LIST 
        "convoAI.c" 
        "cjson/cJSON.c"
        "peripheral/common.c" "peripheral/tf.c" "peripheral/screen.c" "peripheral/audio.c" 
        "font/fang_song.c" "ui/home_page.c" 
        "net/wifi.c" "net/http_server.c" "net/http_client.c")

idf_component_register(SRCS ${SRC_LIST}
                    INCLUDE_DIRS "." "cjson" "peripheral" "ui" "net")
                    # REQUIRES esp_lcd lvgl)
