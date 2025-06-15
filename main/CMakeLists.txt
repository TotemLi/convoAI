idf_component_register(SRCS "convoAI.c" "peripheral/common.c" "peripheral/tf.c" "peripheral/screen.c" "peripheral/audio.c"
                    INCLUDE_DIRS "." "peripheral")
                    # REQUIRES esp_lcd lvgl)
