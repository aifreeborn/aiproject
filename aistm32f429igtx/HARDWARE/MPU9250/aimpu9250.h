#ifndef AI_MP9250_H_
#define AI_MP9250_H_


/*
********************************************************************************
*                          INCLUDE HEADER FILES
********************************************************************************
*/
#include "aitypes.h"

/*
********************************************************************************
*                          EXPORTED DEFINE
********************************************************************************
*/
// 如果AD0脚(9脚)接地,IIC地址为0X68(不包含最低位).
// 如果接V3.3,则IIC地址为0X69(不包含最低位).
#define    AI_MPU9250_ADDR       0x68        // MPU6500的器件IIC地址
#define    AI_MPU9250_ID1        0x71        // MPU6500的器件ID1
#define    AI_MPU9250_ID2        0x72

// MPU9250内部封装了一个AK8963磁力计,地址和ID如下
#define    AI_AK8963_ADDR        0x0c        // AK8963的I2C地址
#define    AI_AK8963_ID          0x48        // AK8963的器件ID

// Register Map for Magnetometer
#define    AI_MPU9250_REG_MAG_WIA        0x00        // Magnetometer Device ID
#define    AI_MPU9250_REG_MAG_INFO       0x01
#define    AI_MPU9250_REG_MAG_ST1        0x02
#define    AI_MPU9250_REG_MAG_HXL        0x03
#define    AI_MPU9250_REG_MAG_HXH        0x04
#define    AI_MPU9250_REG_MAG_HYL        0x05
#define    AI_MPU9250_REG_MAG_HYH        0x06
#define    AI_MPU9250_REG_MAG_HZL        0x07
#define    AI_MPU9250_REG_MAG_HZH        0x08
#define    AI_MPU9250_REG_MAG_ST2        0x09
#define    AI_MPU9250_REG_MAG_CNTL1      0x0a
#define    AI_MPU9250_REG_MAG_CNTL2      0x0b
#define    AI_MPU9250_REG_MAG_ASTC       0x0c
#define    AI_MPU9250_REG_MAG_TS1        0x0d
#define    AI_MPU9250_REG_MAG_TS2        0x0e
#define    AI_MPU9250_REG_MAG_I2CDIS     0x0f
#define    AI_MPU9250_REG_MAG_ASAX       0x10
#define    AI_MPU9250_REG_MAG_ASAY       0x11
#define    AI_MPU9250_REG_MAG_ASAZ       0x12

// Register Map for Gyroscope and Accelerometer
#define    AI_MPU9250_REG_SELF_TEST_X_GYRO    0x00
#define    AI_MPU9250_REG_SELF_TEST_Y_GYRO    0x01
#define    AI_MPU9250_REG_SELF_TEST_Z_GYRO    0x02
#define    AI_MPU9250_REG_SELF_TEST_X_ACCEL   0x0d
#define    AI_MPU9250_REG_SELF_TEST_Y_ACCEL   0x0e
#define    AI_MPU9250_REG_SELF_TEST_Z_ACCEL   0x0f
#define    AI_MPU9250_REG_XG_OFFSET_H         0x13
#define    AI_MPU9250_REG_XG_OFFSET_L         0x14
#define    AI_MPU9250_REG_YG_OFFSET_H         0x15
#define    AI_MPU9250_REG_YG_OFFSET_L         0x16
#define    AI_MPU9250_REG_ZG_OFFSET_H         0x17
#define    AI_MPU9250_REG_ZG_OFFSET_L         0x18
#define    AI_MPU9250_REG_SMPLRT_DIV          0x19
#define    AI_MPU9250_REG_CFG                 0x1a
#define    AI_MPU9250_REG_GRYO_CFG            0x1b
#define    AI_MPU9250_REG_ACCEL_CFG           0x1c
#define    AI_MPU9250_REG_ACCEL_CFG2          0x1d
#define    AI_MPU9250_REG_LP_ACCEL_ODR        0x1e
#define    AI_MPU9250_REG_WOM_THR             0x1f
#define    AI_MPU9250_REG_FIFO_EN             0x23
#define    AI_MPU9250_REG_I2C_MST_CTRL        0x24
#define    AI_MPU9250_REG_I2C_SLV0_ADDR       0x25
#define    AI_MPU9250_REG_I2C_SLV0            0x26
#define    AI_MPU9250_REG_I2C_SLV0_CTRL       0x27
#define    AI_MPU9250_REG_I2C_SLV1_ADDR       0x28
#define    AI_MPU9250_REG_I2C_SLV1            0x29
#define    AI_MPU9250_REG_I2C_SLV1_CTRL       0x2a
#define    AI_MPU9250_REG_I2C_SLV2_ADDR       0x2b
#define    AI_MPU9250_REG_I2C_SLV2            0x2c
#define    AI_MPU9250_REG_I2C_SLV2_CTRL       0x2d
#define    AI_MPU9250_REG_I2C_SLV3_ADDR       0x2e
#define    AI_MPU9250_REG_I2C_SLV3            0x2f
#define    AI_MPU9250_REG_I2C_SLV3_CTRL       0x30
#define    AI_MPU9250_REG_I2C_SLV4_ADDR       0x31
#define    AI_MPU9250_REG_I2C_SLV4            0x32
#define    AI_MPU9250_REG_I2C_SLV4_DO         0x33
#define    AI_MPU9250_REG_I2C_SLV4_CTRL       0x34
#define    AI_MPU9250_REG_I2C_SLV4_DI         0x35
#define    AI_MPU9250_REG_I2C_MST_STAT        0x36
#define    AI_MPU9250_REG_INT_PIN_CFG         0x37
#define    AI_MPU9250_REG_INT_EN              0x38
#define    AI_MPU9250_REG_STAT                0x3a
#define    AI_MPU9250_REG_ACCEL_XOUT_H        0x3b
#define    AI_MPU9250_REG_ACCEL_XOUT_L        0x3c
#define    AI_MPU9250_REG_ACCEL_YOUT_H        0x3d
#define    AI_MPU9250_REG_ACCEL_YOUT_L        0x3e
#define    AI_MPU9250_REG_ACCEL_ZOUT_H        0x3f
#define    AI_MPU9250_REG_ACCEL_ZOUT_L        0x40
#define    AI_MPU9250_REG_TEMP_OUT_H          0x41
#define    AI_MPU9250_REG_TEMP_OUT_L          0x42
#define    AI_MPU9250_REG_GYRO_XOUT_H         0x43
#define    AI_MPU9250_REG_GYRO_XOUT_L         0x44
#define    AI_MPU9250_REG_GYRO_YOUT_H         0x45
#define    AI_MPU9250_REG_GYRO_YOUT_L         0x46
#define    AI_MPU9250_REG_GYRO_ZOUT_H         0x47
#define    AI_MPU9250_REG_GYRO_ZOUT_L         0x48
#define    AI_MPU9250_REG_EXT_SENS_DATA_00    0x49
#define    AI_MPU9250_REG_EXT_SENS_DATA_01    0x4a
#define    AI_MPU9250_REG_EXT_SENS_DATA_02    0x4b
#define    AI_MPU9250_REG_EXT_SENS_DATA_03    0x4c
#define    AI_MPU9250_REG_EXT_SENS_DATA_04    0x4d
#define    AI_MPU9250_REG_EXT_SENS_DATA_05    0x4e
#define    AI_MPU9250_REG_EXT_SENS_DATA_06    0x4f
#define    AI_MPU9250_REG_EXT_SENS_DATA_07    0x50
#define    AI_MPU9250_REG_EXT_SENS_DATA_08    0x51
#define    AI_MPU9250_REG_EXT_SENS_DATA_09    0x52
#define    AI_MPU9250_REG_EXT_SENS_DATA_10    0x53
#define    AI_MPU9250_REG_EXT_SENS_DATA_11    0x54
#define    AI_MPU9250_REG_EXT_SENS_DATA_12    0x55
#define    AI_MPU9250_REG_EXT_SENS_DATA_13    0x56
#define    AI_MPU9250_REG_EXT_SENS_DATA_14    0x57
#define    AI_MPU9250_REG_EXT_SENS_DATA_15    0x58
#define    AI_MPU9250_REG_EXT_SENS_DATA_16    0x59
#define    AI_MPU9250_REG_EXT_SENS_DATA_17    0x5a
#define    AI_MPU9250_REG_EXT_SENS_DATA_18    0x5b
#define    AI_MPU9250_REG_EXT_SENS_DATA_19    0x5c
#define    AI_MPU9250_REG_EXT_SENS_DATA_20    0x5e
#define    AI_MPU9250_REG_EXT_SENS_DATA_21    0x5e
#define    AI_MPU9250_REG_EXT_SENS_DATA_22    0x5f
#define    AI_MPU9250_REG_EXT_SENS_DATA_23    0x60
#define    AI_MPU9250_REG_I2C_SLV0_DO         0x63
#define    AI_MPU9250_REG_I2C_SLV1_DO         0x64
#define    AI_MPU9250_REG_I2C_SLV2_DO         0x65
#define    AI_MPU9250_REG_I2C_SLV3_DO         0x66
#define    AI_MPU9250_REG_I2C_MST_DELAY_CTRL  0x67
#define    AI_MPU9250_REG_SIGNAL_PATH_RST     0x68
#define    AI_MPU9250_REG_MOT_DETECT_CTRL     0x69
#define    AI_MPU9250_REG_USER_CTRL           0x6a
#define    AI_MPU9250_REG_PWR_MGMT1           0x6b
#define    AI_MPU9250_REG_PWR_MGMT2           0x6c
#define    AI_MPU9250_REG_FIFO_COUNTH         0x72
#define    AI_MPU9250_REG_FIFO_COUNTL         0x73
#define    AI_MPU9250_REG_FIFO_R_W            0x74
#define    AI_MPU9250_REG_WHO_AM_I            0x75
#define    AI_MPU9250_REG_XA_OFFSET_H         0x77
#define    AI_MPU9250_REG_XA_OFFSET_L         0x78
#define    AI_MPU9250_REG_YA_OFFSET_H         0x7a
#define    AI_MPU9250_REG_YA_OFFSET_L         0x7b
#define    AI_MPU9250_REG_ZA_OFFSET_H         0x7d
#define    AI_MPU9250_REG_ZA_OFFSET_L         0x7e

/*
********************************************************************************
*                           FUNCTION PROTOTYPES
********************************************************************************
*/
int ai_mpu9250_init(void);
int ai_mpu9250_write_byte(u8 addr, u8 reg, u8 data);
u8 ai_mpu9250_read_byte(u8 addr, u8 reg);
int ai_mpu9250_write(u8 addr, u8 reg, u8 len, u8 *buf);
int ai_mpu9250_read(u8 addr, u8 reg, u8 len, u8 *buf);

int ai_mpu9250_set_gyro_fsr(u8 fsr);
int ai_mpu9250_set_accel_fsr(u8 fsr);
int ai_mpu9250_set_lpf(u16 lpf);
int ai_mpu9250_set_rate(u16 rate);
float ai_mpu9250_get_temperature(void);
int ai_mpu9250_get_gyroscope(short *gx, short *gy, short *gz);
int ai_mpu9250_get_accelerometer(short *ax, short *ay, short *az);
int ai_mpu9250_get_magnetometer(short *mx, short *my, short *mz);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AI_MP9250_H_ */
