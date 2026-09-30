#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "hal/gpio_hal.h"
#include "soc/gpio_reg.h" // Нужен для доступа к адресам регистров

#define WIFI_SSID      "HAPPY_BIRTHDAY"  // Имя Wi-Fi сети, которую создаст ESP32
#define WIFI_PASS      "12345678"
#define BLINK_GPIO     2   // Встроенный светодиод на плате ESP32

static const char *TAG = "DR_Firmware";
static bool party_mode = false;

// Поток во FreeRTOS, который просто мигает светодиодом, если включен режим пати


static void led_party_task(void *pvParameters)
{
    // Настраиваем GPIO 2 на вывод (включаем 2-й бит в регистре ENABLE)
    REG_WRITE(GPIO_ENABLE_REG, REG_READ(GPIO_ENABLE_REG) | (1 << BLINK_GPIO));
    
    while (1) {
        if (party_mode) {
            // Включаем светодиод (устанавливаем 2-й бит в регистре OUT)
            REG_WRITE(GPIO_OUT_REG, REG_READ(GPIO_OUT_REG) | (1 << BLINK_GPIO));
            vTaskDelay(pdMS_TO_TICKS(100));
            
            // Выключаем светодиод (сбрасываем 2-й бит)
            REG_WRITE(GPIO_OUT_REG, REG_READ(GPIO_OUT_REG) & ~(1 << BLINK_GPIO));
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            REG_WRITE(GPIO_OUT_REG, REG_READ(GPIO_OUT_REG) & ~(1 << BLINK_GPIO));
            vTaskDelay(pdMS_TO_TICKS(250));
        }
    }
}




// Обработчик POST запроса от кнопки (включает мигание на плате)
static esp_err_t action_handler(httpd_req_t *req)
{
    party_mode = true; // Включаем мигалку
    httpd_resp_set_status(req, "200 OK");
    httpd_resp_send(req, "PARTY_STARTED", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Главная страница
static esp_err_t root_get_handler(httpd_req_t *req)
{
    // Новый HTML со встроенным 8-битным синтезатором мелодии Happy Birthday
    const char* resp_str = 
        "<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
        "<title>HB_PROJECT</title>"
        "<style>"
        "body { background: #0a0a0f; color: #00ff66; font-family: monospace; text-align: center; padding: 20px; }"
        ".btn-start { background: #ff0055; color: white; border: none; padding: 20px 40px; font-size: 1.5em; "
        "cursor: pointer; font-weight: bold; border-radius: 10px; box-shadow: 0 0 20px #ff0055; margin-top: 40vh; }"
        ".terminal { display: none; border: 2px solid #00ff66; max-width: 600px; margin: 40px auto; padding: 30px; "
        "background: #111116; border-radius: 8px; box-shadow: 0 0 20px rgba(0,255,102,0.3); text-align: left; }"
        "h1 { color: #ff0055; text-shadow: 0 0 10px #ff0055; margin-top: 0; }"
        ".accent { color: #00ffff; font-weight: bold; }"
        "</style></head><body>"

        "<button id=\"startBtn\" class=\"btn-start\" onclick=\"startEverything()\">ЗАПУСТИТЬ ДВИЖУХУ</button>"

        "<div id=\"payload\" class=\"terminal\">"
        "<h1>[!] SYSTEM_OVERRIDE: SUCCESS</h1>"
        "<p>> <span class=\"accent\">Загрузка ядра...</span> [100%]</p>"
        "<p>> <span class=\"accent\">Статус Свояка:</span> Опыт обновлен до максимальной версии.</p>"
        "<p>> <span class=\"accent\">Обнаружены критические параметры:</span> Ум, Упорство, Харизма.</p>"
        "<p>--------------------------------------------------</p>"
        "<p><b>С Днем Рождения, Хакимчик-красавчик! 🥂</b></p>"
        "<p>Система успешно скомпилировала новый жизненный цикл. Желаю тебе:</p>"
        "<p>— Чтобы все твои идеи компилировались с первого раза без единого варнинга.</p>"
        "<p>— Чтобы сложные алгоритмы сдавались без боя, а память никогда не утекала.</p>"
        "<p>— Пусть в твоем жизненном стеке всегда будет переполнение от крутых проектов, больших денег и кайфового отдыха!</p>"
        "<p>--------------------------------------------------</p>"
        "</div>"


        "<script>"
        "function playTargetMelody() {"
        "  var ctx = new (window.AudioContext || window.webkitAudioContext)();"
        "  var notes = [261, 261, 293, 261, 349, 329, 261, 261, 293, 261, 392, 349, 261, 261, 523, 440, 349, 329, 293, 466, 466, 440, 349, 392, 349];"
        "  var durations = [400, 400, 800, 800, 800, 1200, 400, 400, 800, 800, 800, 1200, 400, 400, 800, 800, 800, 800, 800, 400, 400, 800, 800, 800, 1200];"
        "  var time = ctx.currentTime;"
        "  notes.forEach(function(note, i) {"
        "    var osc = ctx.createOscillator();"
        "    var gain = ctx.createGain();"
        "    osc.type = 'square';"
        "    osc.frequency.value = note;"
        "    gain.gain.setValueAtTime(0.08, time);"
        "    gain.gain.exponentialRampToValueAtTime(0.00001, time + durations[i]/1000 - 0.05);"
        "    osc.connect(gain);"
        "    gain.connect(ctx.destination);"
        "    osc.start(time);"
        "    osc.stop(time + durations[i]/1000);"
        "    time += durations[i]/1000;"
        "  });"
        "}"

        "function startEverything() {"
        "  document.getElementById('startBtn').style.display = 'none';"
        "  document.getElementById('payload').style.display = 'block';"
        "  try { playTargetMelody(); } catch(e) { alert(e); }"
        "  var xhttp = new XMLHttpRequest();"
        "  xhttp.open('POST', '/action', true);"
        "  xhttp.send();"
        "}"
        "</script></body></html>";
    
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}



static const httpd_uri_t root_uri = { .uri = "/", .method = HTTP_GET, .handler = root_get_handler };
static const httpd_uri_t action_uri = { .uri = "/action", .method = HTTP_POST, .handler = action_handler };

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &root_uri);
        httpd_register_uri_handler(server, &action_uri);
        return server;
    }
    return NULL;
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .channel = 1,
            .password = WIFI_PASS,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_softap();
    start_webserver();
    
    // Запускаем бесконечный поток мигания светодиодом
    xTaskCreate(led_party_task, "led_party", 2048, NULL, 5, NULL);
}

