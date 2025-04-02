#ifndef __FMP383C_H
#define __FMP383C_H

/**
 * @brief 应答包格式
 *
 * 该结构体定义了指纹模块应答包的格式，包括密码、命令字、错误代码及数据段等信息
 */
typedef struct
{
    uint32_t password;    // 模块密码
    uint8_t cmd1;         // 命令字1
    uint8_t cmd2;         // 命令字2
    uint32_t errcode;     // 错误代码
    uint8_t *data;        // 数据指针
    uint16_t data_len;    // 数据长度
} fp_ack_struct;

void fingerprint_init(void);

void fingerprint_deinit(void);

void fp_usart_send_array(uint8_t *array, uint16_t len);

void fp_protocol_gen_packet(uint16_t cmd, uint8_t *data, uint8_t data_len, uint8_t *packet);

void fp_led_touch(uint8_t color);

void fp_led_switch(uint8_t on_off, uint8_t color);

void fp_led_blink(uint8_t color, uint8_t on_10ms, uint8_t off_10ms, uint8_t period);

void fp_led_breathe(uint8_t color, uint8_t max_duty, uint8_t min_duty, uint8_t frequency);

void fp_sleep(uint8_t mode);

void fp_enroll(uint8_t reg_idx);

uint8_t fp_enroll_check(void);

uint8_t fp_finger_state(void);

void fp_receive_clear(void);

void fp_send_command(uint16_t cmd, uint8_t* data, uint8_t data_len);

fp_ack_struct fp_get_ack(void);

//user

void  fp_print_recv_buffer(void);

void fp_led_touch(uint8_t color);

uint8_t fp_enroll_confirm(void);

void fp_enroll_save(uint16_t id);

uint16_t fp_enroll_save_check(void);

void fp_enroll_cancel(void);

uint16_t fp_match_REid(void);

uint8_t fp_match(void);

uint32_t fp_clear_feature_one(uint16_t id);

uint32_t fp_clear_feature_block(uint16_t first_id, uint16_t last_id);

uint32_t fp_clear_feature_all(void);

uint32_t fp_check_feature_clear(void);

uint8_t fp_check_feature_exist(uint16_t id);

uint8_t fp_finger_state(void);

void fp_reset(void);

uint16_t fp_template_num(void);

uint16_t fp_get_gain(void);

void fp_sleep(uint8_t mode);

void fp_set_enroll_count(uint8_t count);

void fp_set_baud(uint32_t baud);

void fp_set_password(uint32_t password);

#endif

