#include "FreeRTOS.h"
#include "task.h"
#include "app_motor.h"
#include "execution_task.h"

extern xTaskHandle xTaskMotorHandle;
DCmotorData dcMotor={.status = 0x00,
                        .percent = 25};

void app_DCmotorControl(uint8_t mode, uint8_t adjust)
{
    switch (mode)
    {
    case 0x01:
        switch (adjust)
        {
        case 0x01:
            // 关
            dcMotor.status = 0x00;
            break;

        case 0x02:
            // 开
            dcMotor.status = 0x01;
            break;
        }
        break;
    case 0x02:
        switch (adjust)
        {
        case 0x01:
            // 调小
            dcMotor.percent -= 25;
            break;

        case 0x02:
            // 调大
            dcMotor.percent += 25;
            break;
        }
    }
    xTaskNotifyGive(xTaskMotorHandle);
}
