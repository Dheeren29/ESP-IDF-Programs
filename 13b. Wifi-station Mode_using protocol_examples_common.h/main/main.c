//You must have to include a additional common function line in CMakelists.txt

#include <stdio.h>
#include "protocol_examples_common.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_log.h"

void app_main(void)
{   esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());
}
