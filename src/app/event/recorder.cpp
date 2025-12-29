#include <app/events.h>
#include <app/tasks.h>
#include <app/audio/wav.h>

static size_t minFreeSpaceBytes = 1048576;
AudioSamples audioSamples;
WavRecorder wavRecorder;

void recordEvent(void *param) {
	bool fileOpened = false;
	// Check LittleFS free space
	if (LittleFS.totalBytes() - LittleFS.usedBytes() < minFreeSpaceBytes) { // 1MB
		File root = LittleFS.open("/audio/");
		if (root && root.isDirectory()) {
			File file = root.openNextFile();
			while (file) {
				String fileName = file.name();
				if (fileName.endsWith(".wav")) {
					LittleFS.remove("/audio/" + fileName);
				}
				file = root.openNextFile();
			}
		}
	}

	wavRecorder.init(LittleFS);
	wavRecorder.setMinFreeSpace(minFreeSpaceBytes);
	size_t initialFree = LittleFS.totalBytes() - LittleFS.usedBytes();
	do {
		if (xQueueReceive(audioQueue, &audioSamples, 100) == pdTRUE) {
			if (!audioSamples.stream) {
				if (fileOpened) {
					wavRecorder.stop();
					float duration = wavRecorder.info();
					ESP_LOGI("WAV_HEADER", "Final Duration: %.2f seconds", duration);
					fileOpened = false;
				}
				vTaskDelete(NULL);
				break;
			}

			if (audioSamples.data != nullptr && audioSamples.length > 0){
				// Check free space
				if (wavRecorder.getRecordedBytes() + audioSamples.length > initialFree - minFreeSpaceBytes) { // Ensure at least 1MB free after this chunk
					ESP_LOGW("LittleFS", "Free space low, skipping chunk to prevent overflow");
					delete[] audioSamples.data;
					audioSamples.data = nullptr;
					// send notif to stop audio record
					notification->send(NOTIFICATION_RECORD, 1);
					continue;
				}

				if (audioSamples.stream) {
#if MQTT_ENABLE
					mqttClient.publish(audioSamples.key, audioSamples.data, audioSamples.length);
#else 
					String audioName = String("/audio/rec_")
						+String(audioSamples.key)
						+".wav";
					if (!fileOpened) {
						if (wavRecorder.start(audioName)) {
							fileOpened = true;
						}
					}
					
					if (fileOpened) {
						wavRecorder.processChunk(audioSamples.data, audioSamples.length);
					}
#endif
				}
				delete[] audioSamples.data;
				audioSamples.data = nullptr;
			}
		}
		vTaskDelay(1);
	} while (true);
}