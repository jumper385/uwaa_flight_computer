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

// K_THREAD_STACK_DEFINE(i2c_task_data, 2048);
// K_THREAD_STACK_DEFINE(spi_task_data, 2048);

struct device *baro = DEVICE_DT_GET(DT_NODELABEL(bme280));
struct device *imu = DEVICE_DT_GET(DT_NODELABEL(mpu6050));

struct sensor_value accel[3];
struct sensor_value gyro[3];
struct sensor_value temperature;
struct sensor_value humidity;
struct sensor_value pressure;

int main(void)
{
	spiTask.mp.type = FS_FATFS;
	spiTask.mp.fs_data = &spiTask.fat_fs;
	spiTask.mp.mnt_point = DISK_MOUNT_PT;
	int res;
	res = fs_mount(&spiTask.mp);

	if (res == FR_OK)
	{
		printk("Disk mounted...\n");
	}
	else
	{
		printk("Failed to Mount Disk...\n");
	}

	res = fs_unmount(&spiTask.mp);

	if (res != 0)
	{
		printk("Failed to unmount...\n");
	}

	for (;;)
	{
		int rc = sensor_sample_fetch(baro);
		rc = sensor_sample_fetch(imu);

		rc = sensor_channel_get(baro, SENSOR_CHAN_PRESS, &pressure);
		rc = sensor_channel_get(baro, SENSOR_CHAN_AMBIENT_TEMP, &temperature);
		rc = sensor_channel_get(baro, SENSOR_CHAN_HUMIDITY, &humidity);

		rc = sensor_channel_get(imu, SENSOR_CHAN_ACCEL_XYZ, &accel);
		rc = sensor_channel_get(imu, SENSOR_CHAN_GYRO_XYZ, &gyro);

		printk("%.2f\n", sensor_value_to_float(&accel[0]));

		k_msleep(1000);
	}

	return 0;
}