#ifndef _I2C_TASK_H_
#define _I2C_TASK_H_

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/byteorder.h>

#include "appTask.h"

#define SPI_BUS DT_ALIAS(spibus)

struct I2CTask
{
  struct AppTask super;
  struct sensor_value accel[3];
  struct sensor_value temperature;
  struct sensor_value gyro[3];
  struct sensor_value pressure;
  struct sensor_value humidity;

  struct device *imu;
  struct device *baro;
};

void I2CTask_init(struct I2CTask *task);
void I2CTask_thread(struct I2CTask *task, void *p2, void *p3);
void I2CTask_emit_imu_task(struct I2CTask *task);
void I2CTask_emit_baro_task(struct I2CTask *task);

#endif