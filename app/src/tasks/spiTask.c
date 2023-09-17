#include "SPITask.h"

void SPITask_init(struct SPITask *task)
{
    AppTask_init((struct AppTask *)task);
    task->mp.type = FS_FATFS;
    task->mp.fs_data = &task->fat_fs;
    task->disk_mount_pt = DISK_MOUNT_PT;
    task->disk_pdrv = DISK_DRIVE_NAME;
}

void SPITask_emit_mount_sd(struct SPITask *task)
{
    k_event_post(&task->super.events, 0b1U);
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
            task->mp.mnt_point = task->disk_mount_pt;
            int res = fs_mount(&task->mp);

            if (res == FR_OK)
            {
                printk("Disk mounted...\n");
            }
            else
            {
                printk("Failed to Mount Disk...\n");
            }

            res = fs_unmount(&task->mp);

            if (res == 0)
            {
                printk("Successfully Unmounted...\n");
            }

            k_event_clear(&appTask->events, 0b1U);
        }

        k_yield();
    }
}