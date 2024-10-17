#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <sys/param.h>
//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
//#include "freertos/event_groups.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "cJSON.h"
//#include "esp_system.h"
//#include "esp_wifi.h"
//#include "esp_event.h"
//#include "lwip/err.h"
//#include "lwip/sys.h"
//#include "esp_spiffs.h"
//#include "esp_http_server.h"

char *str;
static const char *TAG = "Capstone Project";

void write_to_nvs(char* ssid_write, char* pass_write)
{   printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {   printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {   printf("Done\n");
        printf("Updating SSID and pasword to NVS ... ");
        cJSON *root;
        root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "SSID", ssid_write);
        cJSON_AddStringToObject(root, "Password", pass_write);
        const char *my_json_string = cJSON_Print(root);
        err = nvs_set_str(my_handle, "WifiCred", my_json_string);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
        cJSON_Delete(root);
        err = nvs_commit(my_handle);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        nvs_close(my_handle);
    }
}

void read_from_nvs()
{   printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {   printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {   printf("Done\n");
        size_t len;
        err = nvs_get_str(my_handle, "WifiCred", NULL, &len);
        str = (char *)malloc(len);
        err = nvs_get_str(my_handle, "WifiCred", str, &len);
        switch (err)
        {   case ESP_OK:
                printf("Done\n");
                printf("Wifi Credentials = %s\n", str);
                printf("\ncred printed\n");
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                printf("The value is not initialized yet!\n");
                break;
            default:
                printf("Error (%s) reading!\n", esp_err_to_name(err));
        }
        nvs_close(my_handle);

        //cJSON *root2 = cJSON_Parse(str);
        //char *ssidnvs = cJSON_GetObjectItem(root2, "SSID")->valuestring;            //storing the credentials to later use in wifi_init_sta();
        //char *passwordnvs = cJSON_GetObjectItem(root2, "Password")->valuestring;
        //ESP_LOGI(TAG, "SSID=%s", ssidnvs);
        //ESP_LOGI(TAG, "Password=%s", passwordnvs);
        //wifi_init_sta(ssidnvs, passwordnvs);
    }
}

void app_main(void)
{   nvs_flash_init();                       //to initialize the default nvs partition
    write_to_nvs("Kanha 5G", "Airtel@123");
    read_from_nvs();  
}
