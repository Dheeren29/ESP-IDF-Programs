#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "cJSON.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_mac.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_spiffs.h"
#include "esp_http_server.h"

char *str;
static const char *TAG = "Capstone Project";

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;

void wifi_init_sta(char* ssid_connect, char* pass_connect);         //function definition

void write_to_nvs(char* ssid_write, char* pass_write)
{   printf("\n");
    printf("Opening to WRITE Non-Volatile Storage (NVS) handle...\n");
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {   printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {   printf("Updating SSID and pasword to NVS ... ");
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

esp_err_t get_handler(httpd_req_t *req)
{   FILE* f = fopen("/spiffs/ConnectWifi.html", "r");
        if (f == NULL) 
        {   ESP_LOGE(TAG, "Failed to open ConnectWifi.html");
                return ESP_FAIL; }

    char buf[1024];
    memset(buf, 0, sizeof(buf));
    fread(buf, 1, sizeof(buf), f);
    fclose(f);

    httpd_resp_set_type(req, "text/html");
    esp_err_t err= httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);

    if(err == ESP_OK)
        printf("\n On successfully sending the response packet");
    else if(err == ESP_ERR_INVALID_ARG)
        printf("\n Null request pointer");
    else if(err == ESP_ERR_HTTPD_RESP_HDR)
        printf("\n Essential headers are too large for internal buffer");
    else if(err == ESP_ERR_HTTPD_RESP_SEND)
        printf("\n Error in raw send");
    else
        printf("\n Invalid request");

    return ESP_OK;
}

esp_err_t post_handler(httpd_req_t *req)
{   char content[100];
    size_t recv_size = MIN(req->content_len, sizeof(content));
    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) 
    {   if (ret == HTTPD_SOCK_ERR_TIMEOUT) 
        {   httpd_resp_send_408(req);   }
        return ESP_FAIL;
    }
    httpd_resp_send(req, content, HTTPD_RESP_USE_STRLEN);
    
    cJSON *root2 = cJSON_Parse(content);
    char *ssidnvs = cJSON_GetObjectItem(root2, "SSID")->valuestring;
    char *passwordnvs = cJSON_GetObjectItem(root2, "Password")->valuestring;
    printf("Web data received as SSID= %s Password= %s",ssidnvs, passwordnvs );

    write_to_nvs(ssidnvs,passwordnvs);
    wifi_init_sta(ssidnvs,passwordnvs);
    return ESP_OK;
}

httpd_uri_t uri_get = { .uri = "/uri",  .method = HTTP_GET,  .handler = get_handler,  .user_ctx = NULL  };
httpd_uri_t get_wificred = { .uri = "/uri", .method = HTTP_POST, .handler = post_handler,  .user_ctx = NULL};

static httpd_handle_t start_webserver(void)
{   httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) 
    {   ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &uri_get);                               // Set URI handlers
        httpd_register_uri_handler(server, &get_wificred);     
    }
    return server;
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,  int32_t event_id, void* event_data)
{   if (event_id == WIFI_EVENT_AP_STACONNECTED) 
    {   wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",  MAC2STR(event->mac), event->aid);
        start_webserver();
    } 
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) 
    {   wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d, reason=%d",  MAC2STR(event->mac), event->aid, event->reason);
    }
}

void wifi_init_softap(void)
{   printf("\nSoftAP mode \n");

    // Check if netif already exists
    esp_netif_t* ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    if (ap_netif) 
    {   printf("Destroying existing AP netif\n");
        esp_netif_destroy_default_wifi(ap_netif);                   // Netif already exists, destroy it before creating a new one
    }
    esp_netif_create_default_wifi_ap();                             //functions used to provide an IP to the devices in AP mode
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {   .ap = { .ssid = "Mittal ESP32AP",   .channel = 1,    .authmode = WIFI_AUTH_OPEN,    .ssid_hidden = 0,
                                            .max_connection = 4,   .beacon_interval = 100
                                            }
                                };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();
    printf("Wifi_init_softap finished. SSID: %s" ,  "Mittal ESP32AP\n");
}

static void event_handler(void* arg, esp_event_base_t event_base,   int32_t event_id, void* event_data)
{   if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {   esp_wifi_connect(); } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {   if (s_retry_num < 5) 
        {   esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");    } 
        else 
        {   xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);  }
        ESP_LOGI(TAG,"connect to the AP fail");
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {   ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(char* ssid_connect, char* pass_connect)
{   printf("\nStation mode \n");
    s_wifi_event_group = xEventGroupCreate();

    // Check if netif already exists
    esp_netif_t* sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (sta_netif) 
    {   printf("Destroying existing STA netif\n");
        esp_netif_destroy_default_wifi(sta_netif);              // Netif already exists, destroy it before creating a new one
    }
    esp_netif_create_default_wifi_sta();                        // functions used to get an IP to the esp in sta mode

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    wifi_config_t wifi_config = {0}; // Zero initialize all struct memberes
    strcpy((char *)wifi_config.sta.ssid, ssid_connect);
    strcpy((char *)wifi_config.sta.password, pass_connect);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) 
    {   printf("Connected to SSID: %s, password: %s\n", ssid_connect, pass_connect); } 
    else if (bits & WIFI_FAIL_BIT) 
    {   printf("Failed to connect to SSID: %s, password: %s\n", ssid_connect, pass_connect); 
        wifi_init_softap();}                                                                        //called the hotspot if credentials are wrong
    else 
    {   printf("UNEXPECTED EVENT"); }

    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    vEventGroupDelete(s_wifi_event_group);
}

void read_from_nvs()
{   printf("\n");
    printf("Opening to READ Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {   printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {   size_t len;
        err = nvs_get_str(my_handle, "WifiCred", NULL, &len);
        str = (char *)malloc(len);
        err = nvs_get_str(my_handle, "WifiCred", str, &len);
        switch (err)
        {   case ESP_OK:
                printf("Wifi Credentials = %s\n", str);
                printf("cred printed\n");
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                printf("The value is not initialized yet!\n");
                break;
            default:
                printf("Error (%s) reading!\n", esp_err_to_name(err));
        }
        nvs_close(my_handle);

        cJSON *root2 = cJSON_Parse(str);
        char *ssidnvs = cJSON_GetObjectItem(root2, "SSID")->valuestring;            //storing the credentials to later use in wifi_init_sta();
        char *passwordnvs = cJSON_GetObjectItem(root2, "Password")->valuestring;
        wifi_init_sta(ssidnvs, passwordnvs);
    }
}

esp_err_t init_fs(void)
{   esp_vfs_spiffs_conf_t conf = {  .base_path = "/spiffs", .partition_label = NULL,
                                    .max_files = 5,         .format_if_mount_failed = false     };
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) 
    {   if (ret == ESP_FAIL) 
            {   ESP_LOGE(TAG, "Failed to mount or format filesystem");  } 
        else if (ret == ESP_ERR_NOT_FOUND) 
            {   ESP_LOGE(TAG, "Failed to find SPIFFS partition");   } 
        else 
            {   ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));    }
        return ESP_FAIL;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) 
        {   ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret)); } 
    else 
        {   ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);    }
    return ESP_OK;
}

void app_main(void)
{   nvs_flash_init();                                   //to initialize the default nvs partition
    ESP_ERROR_CHECK(esp_netif_init());                  // to initialize the networking stack library for wifi, it only called once in a program
    esp_err_t err = esp_event_loop_create_default();
        if(err == ESP_OK)
        {   printf("Success to create event loop\n");    }
        else if(err == ESP_ERR_NO_MEM)
        {   printf("\n Cannot allocate memory for event loops list \n");    }
        else if(err == ESP_ERR_INVALID_STATE)
        {   printf("Event loop already created\n");     }
        else if(err == ESP_FAIL)
        {   printf("\n Failed to create event loop \n"); }
        else
        {   printf("Failed to create event loop \n");   }
    ESP_ERROR_CHECK(init_fs());
    //write_to_nvs("IN1b", "In@india");
    read_from_nvs(); 
}
