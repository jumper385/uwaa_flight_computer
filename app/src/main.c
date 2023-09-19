/**
 * 1. Call I2C Readings
 * 2. Write data to SD Card through SPI on event receive
 * 3. Spit values through CAN Bus on event receive
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/fs.h>
#include <zephyr/storage/disk_access.h>
#include <ff.h>
#include <stdio.h>

#include "./tasks/i2cTask.h"
#include "./tasks/spiTask.h"

char *data[100];

LOG_MODULE_REGISTER(main);

struct I2CTask i2cTask;
struct SPITask spiTask;

K_THREAD_STACK_DEFINE(i2c_task_data, 2048);

struct k_timer sample_timer;

void sample_expiry_fn(struct k_timer *timer)
{
	I2CTask_emit_imu_task(&i2cTask);
	I2CTask_emit_baro_task(&i2cTask);
}

K_TIMER_DEFINE(sample_timer, sample_expiry_fn, NULL);

int main(void)
{
	// Initialize the threads...
	I2CTask_init(&i2cTask);

	i2cTask.super.tid = k_thread_create(
		&i2cTask.super.thread,
		i2c_task_data, K_THREAD_STACK_SIZEOF(i2c_task_data),
		I2CTask_thread, &i2cTask, NULL, NULL, 2, 0, K_NO_WAIT);

	k_timer_start(&sample_timer, K_NO_WAIT, K_MSEC(100));

	// Mount the Drive...
	SPITask_fn_mount_sd(&spiTask);

	for (;;)
	{

		int event = k_event_wait(&i2cTask.super.events, 0b1111U, false, K_NO_WAIT);

		if ((0b100U == (0b100U & event)) != 0U)
		{
			LOG_INF("ax:%.2f; ay:%.2f; az:%.2f;",
					sensor_value_to_float(&i2cTask.accel[0]),
					sensor_value_to_float(&i2cTask.accel[1]),
					sensor_value_to_float(&i2cTask.accel[2]));
			k_event_clear(&i2cTask.super.events, 0b100U);

			sprintf(data, "ax:%.2f; ay:%.2f; az:%.2f;\r\n",
					sensor_value_to_float(&i2cTask.accel[0]),
					sensor_value_to_float(&i2cTask.accel[1]),
					sensor_value_to_float(&i2cTask.accel[2]));

			SPITask_fn_write_sd(&spiTask, data);
		}

		if ((0b1000U == (0b1000U & event)) != 0U)
		{
			LOG_INF("pressure:%.2f; temperature:%.2f; humidity:%.2f;",
					sensor_value_to_float(&i2cTask.pressure),
					sensor_value_to_float(&i2cTask.temperature),
					sensor_value_to_float(&i2cTask.humidity));
			k_event_clear(&i2cTask.super.events, 0b1000U);

			sprintf(data, "pressure:%.2f; temperature:%.2f; humidity:%.2f;",
					sensor_value_to_float(&i2cTask.pressure),
					sensor_value_to_float(&i2cTask.temperature),
					sensor_value_to_float(&i2cTask.humidity));

			SPITask_fn_write_sd(&spiTask, data);
		}

		k_usleep(100);
	}

	return 0;
}