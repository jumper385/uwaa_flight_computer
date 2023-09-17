/**
 * 1. Call I2C Readings
 * 2. Write data to SD Card through SPI on event receive
 * 3. Spit values through CAN Bus on event receive
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>

#include "./tasks/i2cTask.h"
#include "./tasks/spiTask.h"

struct I2CTask i2cTask;
struct SPITask spiTask;

K_THREAD_STACK_DEFINE(i2c_task_data, 2048);
K_THREAD_STACK_DEFINE(spi_task_data, 2048);

int main(void)
{
	// Initialize the threads...
	I2CTask_init(&i2cTask);
	SPITask_init(&spiTask);

	i2cTask.super.tid = k_thread_create(
		&i2cTask.super.thread,
		i2c_task_data, K_THREAD_STACK_SIZEOF(i2c_task_data),
		I2CTask_thread, &i2cTask, NULL, NULL, 100, 0, K_NO_WAIT);

	spiTask.super.tid = k_thread_create(
		&spiTask.super.thread,
		spi_task_data, K_THREAD_STACK_SIZEOF(spi_task_data),
		SPITask_thread, &spiTask, NULL, NULL, 12, 0, K_NO_WAIT);

	// test: mount the SD Card
	SPITask_emit_mount_sd(&spiTask); // comment this out to log the imu data...

	for (;;)
	{
		// Sample from the Sensors
		I2CTask_emit_imu_task(&i2cTask);
		I2CTask_emit_baro_task(&i2cTask);
		// TODO: add csv logging function

		// Print output for now
		printk("PRESS: %.2f TEMP : %.2f ACCEL_X %.2f\n",
			   sensor_value_to_float(&i2cTask.pressure),
			   sensor_value_to_float(&i2cTask.temperature),
			   sensor_value_to_float(&i2cTask.accel[0]));

		k_msleep(1000);
	}

	return 0;
}