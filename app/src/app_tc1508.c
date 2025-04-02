#include "FreeRTOS.h"
#include "task.h"
#include "app_tc1508.h"
#include "execution_task.h"

extern xTaskHandle xTaskStepperHandle;
tc1508Data_Stepper tc1508_stepper={.status = 0x01,
                                    .direction = 0,
                                    .step = 0,
                                    .angle = 5400,
                                    .delay = 2000};
tc1508Data_DC tc1508_dc;

void app_tc1508Control(uint8_t mode, uint8_t adjust)
{
    switch (mode)
    {
    case 0x01:
        switch (adjust)
        {
        case 0x01:
            // 关
            tc1508_stepper.direction = 0;
            tc1508_stepper.angle = 5400;
            break;

        case 0x02:
            // 开
            tc1508_stepper.direction = 1;
            tc1508_stepper.angle = 5400;
            break;
        }
        break;

    case 0x02:
        switch (adjust)
        {
        case 0x01:
            // 左转
            tc1508_stepper.direction = 0;
            tc1508_stepper.angle = 1800;
            break;

        case 0x02:
            // 右转
            tc1508_stepper.direction = 1;
            tc1508_stepper.angle = 1800;
            break;
        }
        break;

    case 0x03:
        switch (adjust)
        {
        case 0x01:
            vTaskDelay(pdMS_TO_TICKS(5000));
            // 关
            tc1508_stepper.direction = 0;
            tc1508_stepper.angle = 5400;
            break;

        case 0x02:
            vTaskDelay(pdMS_TO_TICKS(5000));
            // 开
            tc1508_stepper.direction = 1;
            tc1508_stepper.angle = 5400;
            break;
        }
        break;
    }
    xTaskNotifyGive(xTaskStepperHandle);
}
