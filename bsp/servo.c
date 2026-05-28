#include "stm32f1xx_hal.h"
#include "pwm.h"

void servo_init() {
    pwm_init(72 - 1, 20000 - 1);
}

void servo_set_angle(float angle) {
    if (angle > 180) {
        angle = 180;
    } else if (angle < 0.0) {
        angle = 0;
    }

    pwm_set_compare(TIM_CHANNEL_2, angle * 2000 / 180 + 500);
}
