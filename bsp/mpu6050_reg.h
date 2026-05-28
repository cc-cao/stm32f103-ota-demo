#ifndef __MPU6050_REG_H
#define __MPU6050_REG_H

// MPU6050 设备地址（写地址，默认 AD0 = 0）
#define MPU6050_ADDR        0x68

// 电源管理
#define MPU6050_PWR_MGMT_1  0x6B
#define MPU6050_PWR_MGMT_2  0x6C

// 采样率分频
#define MPU6050_SMPLRT_DIV  0x19

// 配置寄存器（低通滤波）
#define MPU6050_CONFIG      0x1A

// 陀螺仪配置
#define MPU6050_GYRO_CONFIG 0x1B

// 加速度计配置
#define MPU6050_ACCEL_CONFIG 0x1C

// 加速度数据
#define MPU6050_ACCEL_XOUT_H 0x3B
#define MPU6050_ACCEL_XOUT_L 0x3C
#define MPU6050_ACCEL_YOUT_H 0x3D
#define MPU6050_ACCEL_YOUT_L 0x3E
#define MPU6050_ACCEL_ZOUT_H 0x3F
#define MPU6050_ACCEL_ZOUT_L 0x40

// 温度数据
#define MPU6050_TEMP_OUT_H   0x41
#define MPU6050_TEMP_OUT_L   0x42

// 陀螺仪数据
#define MPU6050_GYRO_XOUT_H  0x43
#define MPU6050_GYRO_XOUT_L  0x44
#define MPU6050_GYRO_YOUT_H  0x45
#define MPU6050_GYRO_YOUT_L  0x46
#define MPU6050_GYRO_ZOUT_H  0x47
#define MPU6050_GYRO_ZOUT_L  0x48

// 信号路径复位
#define MPU6050_SIGNAL_PATH_RESET 0x68

// 设备ID
#define MPU6050_WHO_AM_I    0x75

#endif /* __MPU6050_REG_H */
