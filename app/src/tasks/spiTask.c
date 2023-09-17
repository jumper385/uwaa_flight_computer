#include "SPITask.h"

void SPITask_init(struct SPITask *task)
{
    AppTask_init((struct AppTask *)task);
    task->mp.type = FS_FATFS;
    task->mp.fs_data = &task->fat_fs;
    task->disk_mount_pt = DISK_MOUNT_PT;
    task->disk_pdrv = DISK_DRIVE_NAME;
}

int lsdir(const char *path)
{
    int res;
    struct fs_dir_t dirp;
    static struct fs_dirent entry;

    fs_dir_t_init(&dirp);

    /* Verify fs_opendir() */
    res = fs_opendir(&dirp, path);
    if (res)
    {
        printk("Error opening dir %s [%d]\n", path, res);
        return res;
    }

    printk("\nListing dir %s ...\n", path);
    for (;;)
    {
        /* Verify fs_readdir() */
        res = fs_readdir(&dirp, &entry);

        /* entry.name[0] == 0 means end-of-dir */
        if (res || entry.name[0] == 0)
        {
            break;
        }

        if (entry.type == FS_DIR_ENTRY_DIR)
        {
            printk("[DIR ] %s\n", entry.name);
        }
        else if (entry.type == FS_DIR_ENTRY_FILE)
        {
            printk("[FILE ] %s\n", entry.name);
        }
        else
        {
            printk("[FILE] %s (size = %zu)\n",
                   entry.name, entry.size);
        }
    }

    /* Verify fs_closedir() */
    fs_closedir(&dirp);

    return res;
}

int write_file(const char *filename, const char *data)
{
    int res;
    struct fs_file_t filep;
    static struct fs_dirent entry;

    fs_file_t_init(&filep);

    res = fs_open(&filep, filename, FS_O_CREATE | FS_O_RDWR | FS_O_APPEND);

    if (res != 0)
    {
        printk("failed to open file... %s, %d\n", filename, res);
    }

    res = fs_write(&filep, data, strlen(data));

    if (res < 0)
    {
        printk("Failed to write...\n");
    }

    res = fs_close(&filep);
    if (res < 0)
    {
        printk("FAIL: close %s: %d\n", filename, res);
    }
    return res;
}

void SPITask_emit_mount_sd(struct SPITask *task)
{
    k_event_post(&task->super.events, 0b1U);
    return;
}

void SPITask_emit_read_sd(struct SPITask *task)
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

            k_event_clear(&appTask->events, 0b1U);
        }

        if ((0b10U == (0b10U & event)) != 0U)
        {
            char *fname = "/SD:/test.txt";
            char data[100] = "JUMPERSSS\r\n";
            sprintf(data, "[%lld] TESTING %.2f\r\n", k_uptime_get(), 0.245);

            write_file(fname, &data);
            // lsdir(task->disk_mount_pt);
            k_event_clear(&appTask->events, 0b10U);
        }

        k_yield();
    }
}