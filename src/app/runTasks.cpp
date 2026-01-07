#include "tasks.h"
#include <WebSocketClientSSL.h>

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

  // xTaskCreate([](void* param){
  //   while(!wifiManager.isConnected()) vTaskDelay(10000);

  //   WebSocketClientSSL wss;
  //   String url = "/v1/realtime?model=gpt-realtime-mini";
  //   String authHeader = String("Bearer ") + GPT_API_KEY;
  //   wss.setAuthorization(authHeader.c_str());
  //   wss.connect("api.openai.com", 443, url.c_str());
    
  //   GPTSpiJsonDocument doc;
  //   doc["type"] = "session.update";
  //   doc["session"]["type"] = "realtime";
  //   doc["session"]["max_output_tokens"] = 1024;

  //   // Output modality
  //   doc["session"]["output_modalities"][0] = "audio";

  //   // Instructions
  //   doc["session"]["instructions"] =
  //       "You are a calm, monotone AI assistant. "
  //       "Speak in short, efficient sentences. "
  //       "Avoid emotional language. "
  //       "Report confidence or probability only when it is relevant. "
  //       "Use dry, understated humor. "
  //       "Maintain a robotic, professional tone at all times. "
  //       "Include humor most of the time. "
  //       "Include numeric confidence occasionally, but only if relevant. "
  //       "If your answer is long, break it into multiple short statements.";

  //   // Audio input config
  //   doc["session"]["audio"]["input"]["format"]["type"] = "audio/pcm";
  //   doc["session"]["audio"]["input"]["format"]["rate"] = 24000;
  //   doc["session"]["audio"]["input"]["noise_reduction"]["type"] = "near_field";

  //   // Transcription config
  //   doc["session"]["audio"]["input"]["transcription"]["model"] = "gpt-4o-mini-transcribe";

  //   // Turn detection / VAD
  //   doc["session"]["audio"]["input"]["turn_detection"]["type"] = "server_vad";
  //   doc["session"]["audio"]["input"]["turn_detection"]["interrupt_response"] = false;
  //   doc["session"]["audio"]["input"]["turn_detection"]["prefix_padding_ms"] = 300;
  //   doc["session"]["audio"]["input"]["turn_detection"]["silence_duration_ms"] = 3000;
  //   doc["session"]["audio"]["input"]["turn_detection"]["threshold"] = 0.5;

  //   // Audio output config (voice + format)
  //   doc["session"]["audio"]["output"]["format"]["type"] = "audio/pcm";
  //   doc["session"]["audio"]["output"]["format"]["rate"] = 24000;
  //   doc["session"]["tool_choice"] = "auto";

  //   String config;
  //   serializeJson(doc, config);
  //   wss.sendMessage(config);
  //   wss.sendMessage("{\"type\":\"response.create\"}");

  //   do{
  //     auto result = wss.receiveMessage();
  //     if (result) {
  //       ESP_LOGI("WSS", "%s", (char*)result);
  //       heap_caps_free(result);
  //     }
  //   }while(wss.isConnected());

  //   vTaskDelete(NULL);
  // }, "test", 1024 * 8, nullptr, 0, nullptr);

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