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

	struct gpio_dt_spec red_led = GPIO_DT_SPEC_GET(DT_NODELABEL(led_red), gpios);
	int ret = gpio_pin_configure_dt(&red_led, GPIO_OUTPUT_INACTIVE);

	SPITask_fn_mount_sd(&spiTask);

	ret = gpio_pin_set_dt(&red_led, 1);

	for (;;)
	{

		int event = k_event_wait(&i2cTask.super.events, 0b1111U, false, K_NO_WAIT);

		if ((0b100U == (0b100U & event)) != 0U)
		{
			LOG_INF("%lld,%s,%.3f,%.3f,%.3f,%3f,%3f,%3f\r\n", k_uptime_get(), "IMU",
					sensor_value_to_double(&i2cTask.accel[0]),
					sensor_value_to_double(&i2cTask.accel[1]),
					sensor_value_to_double(&i2cTask.accel[2]),
					sensor_value_to_double(&i2cTask.gyro[0]),
					sensor_value_to_double(&i2cTask.gyro[1]),
					sensor_value_to_double(&i2cTask.gyro[2]));

			char buf[100];
			sprintf(buf, "%lld,%s,%.3f,%.3f,%.3f,%3f,%3f,%3f\r\n", k_uptime_get(), "IMU",
					sensor_value_to_double(&i2cTask.accel[0]),
					sensor_value_to_double(&i2cTask.accel[1]),
					sensor_value_to_double(&i2cTask.accel[2]),
					sensor_value_to_double(&i2cTask.gyro[0]),
					sensor_value_to_double(&i2cTask.gyro[1]),
					sensor_value_to_double(&i2cTask.gyro[2]));

			SPITask_fn_write_sd(&spiTask, &buf, "/SD:/imu.csv");

			k_event_clear(&i2cTask.super.events, 0b100U);
		}

		if ((0b1000U == (0b1000U & event)) != 0U)
		{
			LOG_INF("%lld,%s,%.3f,%.3f,%.3f\r\n", k_uptime_get(), "BARO",
					sensor_value_to_double(&i2cTask.pressure),
					sensor_value_to_double(&i2cTask.temperature),
					sensor_value_to_double(&i2cTask.humidity));

			char buf[100];
			sprintf(buf, "%lld,%s,%.8f,%.3f,%.3f\r\n", k_uptime_get(), "BARO",
					sensor_value_to_double(&i2cTask.pressure),
					sensor_value_to_double(&i2cTask.temperature),
					sensor_value_to_double(&i2cTask.humidity));

			SPITask_fn_write_sd(&spiTask, &buf, "/SD:/baro.csv");

			k_event_clear(&i2cTask.super.events, 0b1000U);
		}

		k_usleep(100);
	}

	return 0;
}