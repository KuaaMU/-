#include "tc1508.h"

#if STEPPER_MOTOR

static uint32_t     tc1508_beats;
static uint8_t      tc1508_direction;

/**
  * @brief   步进电机按拍数旋转，拍数为 0 ~ 3 或 3 ~ 0 之间循环
  *
  * @param   dir: 方向：1：顺时针，0：逆时针
  * @param  step: 拍数：0 ~ 3 之间
  */
void tc1508_action(uint8_t dir, uint8_t step)
{
    step = (dir == 0) ? (3 - step) : step;

    switch (step) {
        case 0:
            gpio_bit_set(TC1508_PORT, TC1508_PIN_1 | TC1508_PIN_3);
            gpio_bit_reset(TC1508_PORT, TC1508_PIN_2 | TC1508_PIN_4);
            break;
        case 1:
            gpio_bit_set(TC1508_PORT, TC1508_PIN_1 | TC1508_PIN_4);
            gpio_bit_reset(TC1508_PORT, TC1508_PIN_3 | TC1508_PIN_2);
            break;
        case 2:
            gpio_bit_set(TC1508_PORT, TC1508_PIN_2 | TC1508_PIN_4);
            gpio_bit_reset(TC1508_PORT, TC1508_PIN_1 | TC1508_PIN_3);
            break;
        case 3:
            gpio_bit_set(TC1508_PORT, TC1508_PIN_2 | TC1508_PIN_3);
            gpio_bit_reset(TC1508_PORT, TC1508_PIN_1 | TC1508_PIN_4);
            break;
    }
}


/**
  * @brief   步进电机按整数角度旋转
  *
  * @param   direction: 转动方向：1：顺时针(正转)，0：逆时针(反转)
  * @param       angle: 电机转轴旋转角度
  */
void tc1508_angle(uint8_t direction, uint32_t angle)
{
	bsp_timer5_disable();

	tc1508_direction = direction;
	tc1508_beats = (angle * 20) / 360;     // 计算角度对应总的节拍数

	bsp_timer5_enable();
}


/**
  * @brief   定时器更新中断刷新步进电机的节拍
  *
  */
void tc1508_timer5_update(void)
{
    if (tc1508_beats != 0) {
        tc1508_action(tc1508_direction, (3 - tc1508_beats-- % 4));
    } else {
        tc1508_stop();
    }
}


/**
  * @brief   四相五线步进电机引脚配置及初始化
  *
  */
void tc1508_stepper_init(void)
{
		bsp_timer5_init(10000, tc1508_timer5_update);

    rcu_periph_clock_enable(TC1508_RCU);

    gpio_mode_set(TC1508_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, TC1508_PIN_1 | TC1508_PIN_2 | TC1508_PIN_3 | TC1508_PIN_4);
    gpio_output_options_set(TC1508_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, TC1508_PIN_1 | TC1508_PIN_2 | TC1508_PIN_3 | TC1508_PIN_4);

    gpio_bit_reset(TC1508_PORT, TC1508_PIN_1 | TC1508_PIN_2 | TC1508_PIN_3 | TC1508_PIN_4);
}


/**
  * @brief   停止步进电机转动
  *
  */
void tc1508_stop(void)
{
    bsp_timer5_disable();

    tc1508_beats = 0;
		gpio_bit_reset(TC1508_PORT, TC1508_PIN_1 | TC1508_PIN_2 | TC1508_PIN_3 | TC1508_PIN_4);
}


#else

void tc1508_dc_motor_gpio_init(void)
{
	rcu_periph_clock_enable(TC1508_RCU);

	gpio_af_set(TC1508_PORT, GPIO_AF_2, TC1508_PIN_1);
	gpio_af_set(TC1508_PORT, GPIO_AF_2, TC1508_PIN_2);

	gpio_mode_set(TC1508_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, (TC1508_PIN_1|TC1508_PIN_2));

	gpio_output_options_set(TC1508_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, (TC1508_PIN_1|TC1508_PIN_2));
}

void tc1508_dc_motor_timer_init(void)
{
	rcu_periph_clock_enable(TC1508_DC_MOTOR_TIMER_RCU);
	timer_deinit(TC1508_DC_MOTOR_TIMER);

	timer_parameter_struct timer_initpara;
	timer_initpara.prescaler         = 1200 - 1;                      // 时钟预分频数
	timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;            // 对齐模式
	timer_initpara.counterdirection  = TIMER_COUNTER_UP;              // 向上计数模式
	timer_initpara.period            = 100 - 1;              				  // 自动重装载寄存器周期的值(计数值)
	timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;              // 采样分频
	timer_initpara.repetitioncounter = 0;

	timer_init(TC1508_DC_MOTOR_TIMER, &timer_initpara);

	timer_oc_parameter_struct timer_oc_initpara;
	timer_oc_initpara.outputstate  = TIMER_CCX_ENABLE;            		//通道输出状态
	timer_oc_initpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;      		//通道输出极性
	timer_oc_initpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;    		  //通道处于空闲时的输出
	timer_oc_initpara.outputnstate = TIMER_CCXN_ENABLE; 							//互补通道输出状态
  timer_oc_initpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;         // 互补通道输出极性
  timer_oc_initpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;        // 互补通道处于空闲时的输出


	timer_channel_output_config(TC1508_DC_MOTOR_TIMER, TC1508_DC_MOTOR_PIN1_CH, &timer_oc_initpara);
	timer_channel_output_config(TC1508_DC_MOTOR_TIMER, TC1508_DC_MOTOR_PIN2_CH, &timer_oc_initpara);

	timer_channel_output_pulse_value_config(TC1508_DC_MOTOR_TIMER,TC1508_DC_MOTOR_PIN1_CH,50);
	timer_channel_output_pulse_value_config(TC1508_DC_MOTOR_TIMER,TC1508_DC_MOTOR_PIN2_CH,50);

	timer_channel_output_mode_config(TC1508_DC_MOTOR_TIMER, TC1508_DC_MOTOR_PIN1_CH, TIMER_OC_MODE_PWM0);
	timer_channel_output_mode_config(TC1508_DC_MOTOR_TIMER, TC1508_DC_MOTOR_PIN2_CH, TIMER_OC_MODE_PWM0);

	timer_channel_output_shadow_config(TC1508_DC_MOTOR_TIMER, TC1508_DC_MOTOR_PIN1_CH, TIMER_OC_SHADOW_DISABLE);
	timer_channel_output_shadow_config(TC1508_DC_MOTOR_TIMER, TC1508_DC_MOTOR_PIN2_CH, TIMER_OC_SHADOW_DISABLE);

  timer_auto_reload_shadow_enable(TC1508_DC_MOTOR_TIMER);

	timer_primary_output_config(TC1508_DC_MOTOR_TIMER, ENABLE);

	timer_enable(TC1508_DC_MOTOR_TIMER);
}

void tc1508_dc_motor_init(void)
{
	tc1508_dc_motor_gpio_init();
	tc1508_dc_motor_timer_init();
}

void tc1508_dc_motor_control(uint8_t dir,uint16_t speed)
{
	if(dir == 1){

		timer_channel_output_pulse_value_config(TC1508_DC_MOTOR_TIMER,TC1508_DC_MOTOR_PIN1_CH,0);
		timer_channel_output_pulse_value_config(TC1508_DC_MOTOR_TIMER,TC1508_DC_MOTOR_PIN2_CH,speed);

	}else if(dir == 0){
		timer_channel_output_pulse_value_config(TC1508_DC_MOTOR_TIMER,TC1508_DC_MOTOR_PIN1_CH,speed);
		timer_channel_output_pulse_value_config(TC1508_DC_MOTOR_TIMER,TC1508_DC_MOTOR_PIN2_CH,0);
	}
}

void tc1508_dc_motor_stop(void)
{
		timer_channel_output_pulse_value_config(TC1508_DC_MOTOR_TIMER,TC1508_DC_MOTOR_PIN1_CH,0);
		timer_channel_output_pulse_value_config(TC1508_DC_MOTOR_TIMER,TC1508_DC_MOTOR_PIN2_CH,0);
}

#endif

