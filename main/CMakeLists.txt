idf_component_register(SRCS "softap_example_main.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_http_server esp_wifi nvs_flash driver)
