#include "can.h"
#include "log.h"

CAN_HandleTypeDef hcan1;
volatile u8 can_rx_pending;

void can_init() {
    __HAL_RCC_CAN1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

     GPIO_InitTypeDef gpio_def = {
        .Pin = GPIO_PIN_11,
        .Mode = GPIO_MODE_AF_INPUT,
        .Pull = GPIO_PULLUP,
        .Speed = GPIO_SPEED_FREQ_HIGH,
    };
    HAL_GPIO_Init(GPIOA, &gpio_def);

    gpio_def.Mode = GPIO_MODE_AF_PP;
    gpio_def.Pin = GPIO_PIN_12;
    HAL_GPIO_Init(GPIOA, &gpio_def);

    hcan1.Instance = CAN1;
    // hcan1.Init.Mode = CAN_MODE_LOOPBACK;
    hcan1.Init.Mode = CAN_MODE_NORMAL;
    // 125k clk / prescale / all tq
    hcan1.Init.Prescaler = 48; 
    hcan1.Init.TimeSeg1 = CAN_BS1_2TQ;
    hcan1.Init.TimeSeg2 = CAN_BS2_3TQ;
    hcan1.Init.SyncJumpWidth = CAN_SJW_2TQ;
    hcan1.Init.AutoRetransmission = ENABLE;
    hcan1.Init.TransmitFifoPriority = ENABLE;
    hcan1.Init.ReceiveFifoLocked = DISABLE;
    hcan1.Init.AutoWakeUp = ENABLE;
    hcan1.Init.TimeTriggeredMode = DISABLE;
    hcan1.Init.AutoBusOff = DISABLE;
    
    HAL_CAN_Init(&hcan1);

    CAN_FilterTypeDef sFilterConfig;
    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterFIFOAssignment = CAN_FilterFIFO0;
    sFilterConfig.FilterActivation = CAN_FILTER_ENABLE;
    // 配置过滤器 
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    
    HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);

    HAL_CAN_Start(&hcan1);
}

void can_set_filter(CAN_FilterTypeDef* sFilterConfig) {
    HAL_CAN_Stop(&hcan1);
    HAL_CAN_ConfigFilter(&hcan1, sFilterConfig);
    HAL_CAN_Start(&hcan1);
}

void can_write(CANMsg* msg) {
    CAN_TxHeaderTypeDef tx_header = {0};
    uint32_t tx_mailbox = 0;

    if (msg->len > 8) {
        msg->len = 8;
    }

    if (HAL_CAN_GetState(&hcan1) == HAL_CAN_STATE_READY) {
        if (HAL_CAN_Start(&hcan1) != HAL_OK) {
            return;
        }
    }

    tx_header.IDE = msg->ide;

    if (tx_header.IDE == CAN_ID_STD) {
        tx_header.StdId = msg->id;
    } else {
        tx_header.ExtId = (msg->id & 0x1FFFFFFFU);
    }

    tx_header.RTR = msg->rtr;
    tx_header.DLC = msg->len;
    tx_header.TransmitGlobalTime = DISABLE;

    if (HAL_CAN_AddTxMessage(&hcan1, &tx_header, msg->data, &tx_mailbox) != HAL_OK) {
        return;
    }

    while (HAL_CAN_IsTxMessagePending(&hcan1, tx_mailbox) != 0U) {
    }
}

u8 can_rx_flag() {
    u8 fifo_level = (u8)HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0);
    return (can_rx_pending > fifo_level) ? can_rx_pending : fifo_level;
}

u8 can_get_rx_pending() {
    return can_rx_pending;
}

void can_read(CANMsg* msg) {
    CAN_RxHeaderTypeDef pHeader;
    if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &pHeader, msg->data) != HAL_OK) {
        return;
    }
    if (can_rx_pending > 0) {
        can_rx_pending--;
    }
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);

    if (pHeader.IDE == CAN_ID_STD) {
        msg->id = pHeader.StdId;
    } else {
        msg->id = pHeader.ExtId;
    }

    msg->len = pHeader.DLC;
    msg->rtr = pHeader.RTR;
    msg->ide = pHeader.IDE;
}

void can_isr_enable() {
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 5, 0);
    HAL_NVIC_ClearPendingIRQ(USB_LP_CAN1_RX0_IRQn);
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* hcan) {
    if (hcan->Instance == CAN1 && can_rx_pending < 0xFFU) {
        HAL_CAN_DeactivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
        can_rx_pending++;
    }
}

void can_irq_dispatch() {
    HAL_CAN_IRQHandler(&hcan1);
}
