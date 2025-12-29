#include <app/events.h>
#include <app/tasks.h>

AudioSamples audioSamples;
typedef struct WAV_HEADER {
  /* RIFF Chunk Descriptor */
  uint8_t RIFF[4] = {'R', 'I', 'F', 'F'}; // RIFF Header Magic header
  uint32_t ChunkSize;                     // RIFF Chunk Size
  uint8_t WAVE[4] = {'W', 'A', 'V', 'E'}; // WAVE Header
  /* "fmt" sub-chunk */
  uint8_t fmt[4] = {'f', 'm', 't', ' '}; // FMT header
  uint32_t Subchunk1Size = 16;           // Size of the fmt chunk
  uint16_t AudioFormat = 1; // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM
                            // Mu-Law, 258=IBM A-Law, 259=ADPCM
  uint16_t NumOfChan = 1;   // Number of channels 1=Mono 2=Sterio
  uint32_t SamplesPerSec = 16000;   // Sampling Frequency in Hz
  uint32_t bytesPerSec = 16000 * 2; // bytes per second
  uint16_t blockAlign = 2;          // 2=16-bit mono, 4=16-bit stereo
  uint16_t bitsPerSample = 16;      // Number of bits per sample
  /* "data" sub-chunk */
  uint8_t Subchunk2ID[4] = {'d', 'a', 't', 'a'}; // "data"  string
  uint32_t Subchunk2Size;                        // Sampled data length
} wav_hdr;

void recordEvent() {
	if (xQueueReceive(audioQueue, &audioSamples, 0) == pdTRUE) {
		if (audioSamples.data != nullptr && audioSamples.length > 0){
#if MQTT_ENABLE
			mqttClient.publish(audioSamples.key, audioSamples.data, audioSamples.length);
#else 
			String audioName = String("/audio/rec_")+audioSamples.key+".wav";
			if (!LittleFS.exists(audioName)) {
				wav_hdr header;
				header.ChunkSize = audioSamples.length + 36;
				header.Subchunk2Size = audioSamples.length;
				File audioFile = LittleFS.open(audioName, "w", true);
				if (audioFile) {
					audioFile.write((uint8_t*)&header, sizeof(header));
					audioFile.write(audioSamples.data, audioSamples.length);
					audioFile.close();
				}
			}
			
			File audioFile = LittleFS.open(audioName, "a", true);
			if (audioFile) {
				audioFile.seek(audioFile.size());
				audioFile.write(audioSamples.data, audioSamples.length);
				audioFile.close();
			}
#endif
			delete[] audioSamples.data;
			audioSamples.data = nullptr;
		}
	}
}