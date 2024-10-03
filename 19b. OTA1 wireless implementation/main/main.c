#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "protocol_examples_common.h"
#include "string.h"
#include "driver/gpio.h"
#include "nvs.h"
#include "nvs_flash.h"

#define OTA_URL_SIZE 256 
static const char *TAG = "OTA upload demo";

#define LED1 4
#define LED2 16
uint16_t level1 = 1;
uint16_t level2 = 1;

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
        break;
    }
    return ESP_OK;
}

void simple_ota_example_task(void *pvParameter)
{   ESP_LOGI(TAG, "Starting OTA example");

    esp_http_client_config_t config = { .url = CONFIG_EXAMPLE_FIRMWARE_UPGRADE_URL, .cert_pem = (char *)"",
                                        .event_handler = _http_event_handler,   };
    
    esp_https_ota_config_t ota_config = { .http_config = &config,   };
    
    ESP_LOGI(TAG, "Attempting to download update from %s", config.url);

    esp_err_t ret = esp_https_ota(&ota_config);
    if (ret == ESP_OK) 
        {   ESP_LOGI(TAG, "OTA update successful, restart esp after pausing update roll out...");
            //esp_restart();  
        } 
    else 
        {   ESP_LOGE(TAG, "Firmware upgrade failed");   }
    
    while (1) 
    {   vTaskDelay(1000 / portTICK_PERIOD_MS);  }
}

static void Task1code(void * parameter)
{   while (1)
    {   //printf("\n Task1 running on core : %d", xPortGetCoreID());
        level1 = !level1;
        gpio_set_level(LED1, level1);
        vTaskDelay(100/10);
    }
}
static void Task2code(void * parameter)
{   while (1)
    {   //printf("\n Task2 running on core : %d", xPortGetCoreID());
        level2 = !level2;
        gpio_set_level(LED2, level2);
        vTaskDelay(400/10);
    }
}

void app_main(void)
{   gpio_set_direction(LED1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED2, GPIO_MODE_OUTPUT);

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(example_connect());

    xTaskCreate(&simple_ota_example_task, "Next_ota_update_Ready_task", 8192, NULL, 5, NULL);
    xTaskCreatePinnedToCore(Task1code, "Task1", 8192, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(Task2code, "Task2", 8192, NULL, 1, NULL, 1);
}
