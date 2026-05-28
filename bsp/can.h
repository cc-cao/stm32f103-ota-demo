#pragma once
#include "stm32f1xx_hal.h"
#include "utypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    u32 id;
    u32 ide;
    u32 rtr;
    u8 len;
    u8 data[8];
} CANMsg;

void can_init();
void can_set_filter(CAN_FilterTypeDef* sFilterConfig);
u8 can_rx_flag();
u8 can_get_rx_pending();
void can_write(CANMsg* msg);
void can_read(CANMsg* msg);
void can_isr_enable();
void can_irq_dispatch();

#ifdef __cplusplus
}
#endif
