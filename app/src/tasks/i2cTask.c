#include "i2cTask.h"

void I2CTask_init(struct I2CTask *task)
{
    AppTask_init((struct AppTask *)task);
    task->baro = DEVICE_DT_GET(DT_NODELABEL(bme280));
    task->imu = DEVICE_DT_GET(DT_NODELABEL(mpu6050));

    if (!device_is_ready(task->baro))
    {
        printk("failed to initialize baro\n");
        return;
    }

    if (!device_is_ready(task->imu))
    {
        printk("failed to initialize imu\n");
        return;
    }
}

int I2CTask_getImuData(struct I2CTask *task)
{
    int rc = sensor_sample_fetch(task->imu);

    if (rc != 0)
    {
        printk("failed to fetch from IMU\n");
    }

    rc = sensor_channel_get(task->imu, SENSOR_CHAN_ACCEL_XYZ, &task->accel);
    rc = sensor_channel_get(task->imu, SENSOR_CHAN_GYRO_XYZ, &task->gyro);

    if (rc != 0)
    {
        printk("failed to get from IMU\n");
    }

    return rc;
}

int I2CTask_getBaroData(struct I2CTask *task)
{
    int rc = sensor_sample_fetch(task->baro);
    if (rc != 0)
    {
        printk("failed to fetch from baro\n");
    }
    rc = sensor_channel_get(task->baro, SENSOR_CHAN_PRESS, &task->pressure);
    rc = sensor_channel_get(task->baro, SENSOR_CHAN_AMBIENT_TEMP, &task->temperature);
    rc = sensor_channel_get(task->baro, SENSOR_CHAN_HUMIDITY, &task->humidity);

    if (rc != 0)
    {
        printk("failed to get get baro\n");
    }
    return rc;
}

// UserSpace Access
void I2CTask_emit_imu_task(struct I2CTask *task)
{
    k_event_post(&task->super.events, 0b1U);
    return;
}

void I2CTask_emit_baro_task(struct I2CTask *task)
{
    k_event_post(&task->super.events, 0b10U);
    return;
}

void I2CTask_thread(struct I2CTask *task, void *p2, void *p3)
{
    struct AppTask *appTask = (struct AppTask *)task;

    for (;;)
    {
        int event = k_event_wait(&appTask->events, 0b111U, false, K_NO_WAIT);

        if ((0b1U == (0b1U & event)) != 0U)
        {
            int rc = I2CTask_getImuData(task);

            if (rc != 0)
            {
                printk("Failed to get IMU Data...\n");
            }

            k_event_clear(&appTask->events, 0b1U);
            k_event_post(&appTask->events, 0b100U);
        }

        if ((0b10U == (0b10U & event)) != 0U)
        {
            int rc = I2CTask_getBaroData(task);

            if (rc != 0)
            {
                printk("Failed to get Baro Data...\n");
            }

            k_event_clear(&appTask->events, 0b10U);
            k_event_post(&appTask->events, 0b1000U);
        }

        k_usleep(100);
    }
}