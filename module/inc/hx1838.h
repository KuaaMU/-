#ifndef __HX_1838_H
#define __HX_1838_H

#define IR_RCU              RCU_GPIOB
#define IR_PORT             GPIOB
#define IR_PIN              GPIO_PIN_14


/**
typedef struct infrared_data {
    uint8_t address_code;
    uint8_t address_inverse_code;
    uint8_t command_code;
    uint8_t command_inverse_code;
} infrared_data_struct;
*/

typedef void (* infrared_recv_callback)(uint8_t code);

void infrared_init(infrared_recv_callback recv_handler);
void ir_recv_clear(void);
char ir_convert_code(uint8_t code);

#endif
