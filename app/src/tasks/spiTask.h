#ifndef _SPI_TASK_H_
#define _SPI_TASK_H_

#include <stdio.h>
#include <zephyr/fs/fs.h>
#include <zephyr/storage/disk_access.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/usb/usbd.h>
#include <ff.h>

#define DISK_DRIVE_NAME "SD"
#define DISK_MOUNT_PT "/" DISK_DRIVE_NAME ":"

#include "appTask.h"

#define SPI_BUS DT_ALIAS(spibus)

struct SPITask
{
    struct AppTask super;

    // SD/FS Setup Vars
    FATFS fat_fs;
    struct fs_mount_t mp;
    char *disk_mount_pt;
    char *disk_pdrv;
    struct fs_file_t file;

    // data buffers
    char *imu_buf;
    char *baro_buf;
};

void SPITask_fn_mount_sd(struct SPITask *task);
void SPITask_fn_write_sd(struct SPITask *task, char *data, char *fname);

#endif