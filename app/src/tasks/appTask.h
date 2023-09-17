#ifndef __APP_TASK_H_
#define __APP_TASK_H_

#include <zephyr/kernel.h>

struct AppTask {
  struct k_event events;
  k_tid_t tid;
  struct k_thread thread;
};

void AppTask_init(struct AppTask *task);

#endif