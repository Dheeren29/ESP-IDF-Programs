#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "dht.h"

#define DHT_GPIO GPIO_NUM_4  // Change this to the GPIO you're using for the DHT11

void app_main(void)
{   
    int16_t temperature = 0;
    int16_t humidity = 0;
    while (1)
    {   
        if (dht_read_data(DHT_TYPE_DHT11, DHT_GPIO, &humidity, &temperature) == ESP_OK)
        {   printf("Temperature: %d.%d°C", temperature / 10, temperature % 10);
            printf(", Humidity: %d.%d%%\n", humidity / 10, humidity % 10);
        }
        else
        {   printf("Could not read data from sensor\n");
        }
        vTaskDelay(1500/10);  // Wait 1.5 seconds before reading again
    }
}
//Note : There will be an error when we decrease delay between readings.
