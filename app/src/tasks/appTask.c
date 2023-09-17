#include "appTask.h"

void AppTask_init(struct AppTask *task)
{
    k_event_init(&task->events);
}