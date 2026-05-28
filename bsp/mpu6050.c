#include "mpu6050.h"
#include "i2c.h"
#include "mpu6050_reg.h"

#define HI2C  1
#define MPU_ADDR 0xD0

void mpu_regw(uint8_t addr, uint8_t data) {
#if HI2C
    hi2c_reg_write(MPU_ADDR, addr, I2C_MEMADD_SIZE_8BIT, &data, 1);
#else
    i2c_start();
    i2c_send_byte(MPU_ADDR);
    i2c_read_ack();
    i2c_send_byte(addr);
    i2c_read_ack();
    i2c_send_byte(data);
    i2c_read_ack();
    i2c_stop();
#endif
}

uint8_t mpu_regr(uint8_t addr) {
    uint8_t byte;
#if HI2C
    hi2c_reg_read(MPU_ADDR, addr, I2C_MEMADD_SIZE_8BIT, &byte, 1);
#else
    i2c_start();
    i2c_send_byte(MPU_ADDR);
    i2c_read_ack();
    i2c_send_byte(addr);
    i2c_read_ack();

    i2c_start();
    i2c_send_byte(MPU_ADDR | 0x01);
    i2c_read_ack();
    byte = i2c_read_byte();
    i2c_send_ack(1);
    i2c_stop();
#endif

    return byte;
}

void mpu_init() {
#if HI2C
    hi2c_init();
#else
    i2c_init();
#endif
    mpu_regw(MPU6050_PWR_MGMT_1, 0x01);
    mpu_regw(MPU6050_PWR_MGMT_2, 0x00);

    mpu_regw(MPU6050_SMPLRT_DIV, 0x09);
    mpu_regw(MPU6050_CONFIG, 0x06);
    mpu_regw(MPU6050_GYRO_CONFIG, 0x18);
    mpu_regw(MPU6050_ACCEL_CONFIG, 0x18);

}

void mpu_read(MPUData* data) {
    uint8_t dh, dl;
    dh = mpu_regr(MPU6050_ACCEL_XOUT_H);
    dl = mpu_regr(MPU6050_ACCEL_XOUT_L);
    data->accx = (dh << 8) | dl;

    dh = mpu_regr(MPU6050_ACCEL_YOUT_H);
    dl = mpu_regr(MPU6050_ACCEL_YOUT_L);
    data->accy = (dh << 8) | dl;

    dh = mpu_regr(MPU6050_ACCEL_ZOUT_H);
    dl = mpu_regr(MPU6050_ACCEL_ZOUT_L);
    data->accz = (dh << 8) | dl;

    dh = mpu_regr(MPU6050_GYRO_XOUT_H);
    dl = mpu_regr(MPU6050_GYRO_XOUT_L);
    data->gyrox = (dh << 8) | dl;

    dh = mpu_regr(MPU6050_GYRO_YOUT_H);
    dl = mpu_regr(MPU6050_GYRO_YOUT_L);
    data->gyroy = (dh << 8) | dl;

    dh = mpu_regr(MPU6050_GYRO_ZOUT_H);
    dl = mpu_regr(MPU6050_GYRO_ZOUT_L);
    data->gyroz = (dh << 8) | dl;
}

uint8_t mpu_readid() {
    return mpu_regr(MPU6050_WHO_AM_I);
}
