#include "SPITask.h"

void SPITask_init(struct SPITask *task)
{
    AppTask_init((struct AppTask *)task);
}

void SPITask_fn_mount_sd(struct SPITask *task)
{
    static const char *disk_mnt_pt = "/SD:";

    task->mp.type = FS_FATFS;
    task->mp.fs_data = &task->fat_fs;
    task->mp.mnt_point = disk_mnt_pt;

    if (fs_mount(&task->mp) != FR_OK)
    {
        printk("Failed to mount...\n");
    };
}

void SPITask_fn_write_sd(struct SPITask *task, char *data)
{
    fs_file_t_init(&task->file);

    if (fs_open(&task->file, "/SD:/app.txt", FS_O_CREATE | FS_O_RDWR) != 0)
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

void SPITask_emit_mount_sd(struct SPITask *task)
{
    k_event_post(&task->super.events, 0b1U);
    return;
}

void SPITask_emit_write_sd(struct SPITask *task)
{
    k_event_post(&task->super.events, 0b10U);
    return;
}

void SPITask_thread(struct SPITask *task, void *p2, void *p3)
{
    struct AppTask *appTask = (struct AppTask *)task;

    for (;;)
    {
        int event = k_event_wait(&appTask->events, 0b111U, false, K_NO_WAIT);

        if ((0b1U == (0b1U & event)) != 0U)
        {
        }

        if ((0b10U == (0b10U & event)) != 0U)
        {
        }

        k_yield();
    }
}