#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp32/rom/ets_sys.h"

#define DHT11_PIN GPIO_NUM_4                                // GPIO pin where the DHT11 data pin is connected

static const char *TAG = "DHT11";

void DHT11_init(gpio_num_t dht_gpio) 
{   gpio_set_direction(dht_gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(dht_gpio, 1);                            // Initially set to HIGH
}

int DHT11_read(gpio_num_t dht_gpio, int *humidity, int *temperature) 
{
    uint8_t data[5] = {0};
    
    // Send start signal
    gpio_set_direction(dht_gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(dht_gpio, 0);
    vTaskDelay(20/10);                                      // Hold low for at least 18 ms
    gpio_set_level(dht_gpio, 1);
    ets_delay_us(40);
    
    // Switch to input mode to read the response
    gpio_set_direction(dht_gpio, GPIO_MODE_INPUT);
    
    // Wait for DHT11 response
    if (gpio_get_level(dht_gpio) == 1) return ESP_FAIL;
    ets_delay_us(80);
    
    if (gpio_get_level(dht_gpio) == 0) return ESP_FAIL;
    ets_delay_us(80);
    
    // Read 5 bytes of data from the sensor
    for (int i = 0; i < 5; i++) 
    {   for (int j = 0; j < 8; j++) 
        {   while (gpio_get_level(dht_gpio) == 0) {} ;                  // Wait for the pin to go high, then time the low-to-high transition
            ets_delay_us(50);                                           // Wait for 50us
            if (gpio_get_level(dht_gpio) == 1)                          // check whether the bit is 1 or 0, if pin is HIGH its 1 bit
            {   data[i] |= (1 << (7 - j));  }                           // Set the corresponding bit
            while (gpio_get_level(dht_gpio) == 1);                      // Wait for pin to go low
        }
    }
    
    // Check the checksum
    if (data[4] == (data[0] + data[1] + data[2] + data[3])) 
    {   *humidity = data[0];
        *temperature = data[2];
        return ESP_OK;
    } 
    else 
    {   return ESP_FAIL;    }
}

void dht_task(void *pvParameter) 
{   int humidity = 0, temperature = 0;
    while (1) 
    {   if (DHT11_read(DHT11_PIN, &humidity, &temperature) == ESP_OK) 
        {   printf("Temperature: %d°C, Humidity: %d%%\n", temperature, humidity);  } 
        else 
        {   printf("Failed to read from DHT11 sensor");  }
        
        vTaskDelay(1500/10);                                        // Read every 1.5 seconds
    }
}

void app_main(void) 
{
    DHT11_init(DHT11_PIN);
    xTaskCreate(&dht_task, "dht_task", 2048, NULL, 5, NULL);
}
