#include <stdio.h>
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    char c = 0;
    char strArr[100];
    memset(strArr, 0, sizeof(strArr));              //to clear the garbage
    while(c!= '\n')
    {
        c = getchar();
        if(c != 0xff)
        {
            strArr[strlen(strArr)] = c;
            printf("%c", c);
        }
        vTaskDelay(100/portTICK_PERIOD_MS);
    }

    printf("You typed: %s\n", strArr);
}
