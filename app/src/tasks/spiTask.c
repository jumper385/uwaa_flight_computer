#include "SPITask.h"

void SPITask_fn_mount_sd(struct SPITask *task)
{
    do
    {
        static const char *disk_mnt_pt = "/SD:";

        task->mp.type = FS_FATFS;
        task->mp.fs_data = &task->fat_fs;
        task->mp.mnt_point = disk_mnt_pt;

        if (fs_mount(&task->mp) != FR_OK)
        {
            printk("Failed to mount...\n");
        }
        else
        {
            printk("Device Mounted...\n");
            break;
        }

    } while (1);
}

void SPITask_fn_write_sd(struct SPITask *task, char *data, char *fname)
{
    fs_file_t_init(&task->file);

    if (fs_open(&task->file, fname, FS_O_CREATE | FS_O_RDWR) != 0)
    {
        printk("FS Open Failed");
        return;
    }

    if (fs_seek(&task->file, 0, FS_SEEK_END) != 0)
    {
        printk("Failed file Seek");
        return;
    }

    if (fs_write(&task->file, data, strlen(data)) != strlen(data))
    {
        fs_close(&task->file);
        printk("FAILED TO WRITE...");
        return;
    }

    if (fs_sync(&task->file) != 0)
    {
        printk("Failed to SYNC file...\n");
        return;
    }

    fs_close(&task->file);
}