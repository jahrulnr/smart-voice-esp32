#include "tasks.h"

TaskHandle_t taskMonitorerHandle = nullptr;
std::vector<BackgroundTask*> tasks;
QueueHandle_t audioChunkQueue = nullptr;

void runTasks(){
  audioChunkQueue = xQueueCreateWithCaps(20, sizeof(AudioData), MALLOC_CAP_SPIRAM);

  tasks.push_back(new BackgroundTask{
    .name = "mainTask",
    .handle = nullptr,
    .task = mainTask,
    .stack = 1024 * 4,
    .core = 0,
    .priority = 6,
    .caps = MALLOC_CAP_INTERNAL
  });
  tasks.push_back(new BackgroundTask{
    .name = "networkTask",
    .handle = nullptr,
    .task = networkTask,
    .stack = 1024 * 8,
    .core = 1,
    .priority = 1,
    .caps = MALLOC_CAP_INTERNAL,
    .suspendable = true
  });
  // tasks.push_back(new BackgroundTask{
  //   .name = "microphoneTask",
  //   .handle = nullptr,
  //   .task = microphoneTask,
  //   .stack = 1024 * 3,
  //   .core = 1,
  //   .priority = 5,
  //   .caps = MALLOC_CAP_INTERNAL,
  //   .suspendable = true
  // });

  vTaskDelay(100);
  xTaskCreatePinnedToCoreWithCaps(
    taskMonitorer,
    "taskMonitorer",
    1024 * 3,
    NULL,
    0,
    &taskMonitorerHandle,
    1,
    MALLOC_CAP_SPIRAM
  );
}