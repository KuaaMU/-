#include "gd32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"
#include "stdio.h"
// 包含项目特定的头文件
#include "board.h"
#include "flame.h"


// FreeRTOS初始化
/**
 * @brief 初始化FreeRTOS
 *
 * 该函数负责初始化FreeRTOS操作系统，包括创建系统必要的任务和配置。
 */
void FREERTOS_Init(void);

int main(void) {
    // 初始化开发板
    board_init();

    // 初始化FreeRTOS
    FREERTOS_Init();

    // 启动调度器
    /**
     * @brief 启动FreeRTOS任务调度器
     *
     * 该函数调用启动FreeRTOS任务调度器，开始执行创建的任务。
     * 注意：该函数调用后，后续代码不会被执行，除非在任务中调用。
     */
    vTaskStartScheduler();

    // 无限循环，防止main函数退出
    for(;;);
//    return 0;
}
