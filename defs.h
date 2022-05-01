#pragma once

//#define I2C_SDA2 PH_12
//#define I2C_SCL2 PH_11
#define MOTOR_Z_FG PJ_10
#define MOTOR_Z_SPEED PA_8
#define MOTOR_Z_DIR PC_6

#define MOTOR_Y_FG PI_7
#define MOTOR_Y_SPEED PC_7
#define MOTOR_Y_DIR PJ_7

//#define MOTOR_X_FG PI_6
#define MOTOR_X_FG PJ_11
#define MOTOR_X_SPEED PH_6
#define MOTOR_X_DIR PK_1

#define IMUADDRESS1  0x4B    //==75
//#define IMUADDRESS2  0x4A

#define EPS_ADDRESS 0x18
#define IRARRAY_ADDRESS_X1  0x33    //NEVER EVER USER 34 ON TBEAM!!!!@#!@#!@#!@
#define IRARRAY_ADDRESS_X2  0x32
#define IRARRAY_ADDRESS_Y1  0x33
#define IRARRAY_ADDRESS_Y2  0x32
#define IRARRAY_ADDRESS_Z1  0x33
#define IRARRAY_ADDRESS_Z2  0x32
#define TEMP_X1 0x48
#define TEMP_X2 0x49
#define TEMP_Y1 0x48
#define TEMP_Y2 0x49
#define TEMP_Z1 0x48
#define TEMP_Z2 0x49
#define TEMP_OBC 0x4A
//#define TEMP_ADCS 0x4B


#define MAG_ADDRESS_X  0x60
#define MAG_ADDRESS_Y  0x61
#define MAG_ADDRESS_Z  0x63

#define BURNCOUNT_FILE "burncount.txt"
#define DETUMBLE_FILE "dtcount.txt"
#define RS_FILE "rscount.txt"

#define WATCHDOG_LOOP_COUNT 100000
