#include "FreeRTOS.h"
#include "task.h"
#include "app_relay.h"
#include "execution_task.h"

relayData relay1={0x00};
relayData relay2={0x00};

extern TaskHandle_t xTaskRelayHandle;

void app_relayControl(uint8_t device, uint8_t mode, uint8_t adjust)
{
    relayData *relay_temp = NULL;
    switch (device)
    {
    case 1:
        relay_temp = &relay1;
        break;

    case 2:
        relay_temp = &relay2;
        break;
    }

    switch (mode)
    {
    case 0x01:
        switch (adjust)
        {
        case 0x01:
            // 关
            relay_temp->status = 0x00;
            break;

        case 0x02:
            // 开
            relay_temp->status = 0x01;
            break;
        }
        break;
    }
    xTaskNotifyGive(xTaskRelayHandle);
}
