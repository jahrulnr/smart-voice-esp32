#pragma once
#include <app_config.h>
#include <I2SMicrophone.h>
#include <AnalogMicrophone.h>
#include <PDMMicrophone.h>

enum MIC_HW {
	MIC_ANALOG = 0,
	MIC_I2S,
	MIC_PDM,
	MIC_MAX
};

class Microphone {

public:
	Microphone(MIC_HW micType){
		this->micType = micType;
	}

	~Microphone(){
		switch (this->micType) {
		case MIC_ANALOG:
			if (amic) {
				delete amic;
			}
			break;
		case MIC_I2S:
			if (imic) {
				delete imic;
			}
			break;
		case MIC_PDM:
			if (pmic)
				delete pmic;
			break;
		}
	}

	struct Cache {
		int16_t *lastSample = nullptr;
		size_t lastSampleLen = 0;
		unsigned long lastSampleTime = 0;
	};

	inline void init() {
		switch (this->micType) {
		case MIC_ANALOG:
			amic = new AnalogMicrophone(MIC_OUT, MIC_GAIN, MIC_AR);
			amic->init();
			amic->setGain(INPUT);
			amic->setAttackRelease(true);
			amic->start();
			break;
		case MIC_I2S:
			imic = new I2SMicrophone(
					(gpio_num_t)MIC_DIN,    // Data pin
					(gpio_num_t)MIC_SCK,    // Clock pin
					(gpio_num_t)MIC_WS,     // Word select pin
					I2S_NUM_0               // Port number
			);
			imic->init(16000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);
			imic->start();
			break;
		case MIC_PDM:
			pmic = new PDMMicrophone(
					(gpio_num_t)MIC_DIN,    // Data pin
					(gpio_num_t)MIC_SCK,    // Clock pin
					I2S_NUM_0               // Port number
			);
			pmic->init(16000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);
			pmic->start();
			break;
		}
	}

	inline esp_err_t read(void *out, size_t len, size_t *bytes_read, uint32_t timeout_ms){
		esp_err_t ret = ESP_OK;
		switch (this->micType) {
		case MIC_ANALOG: {
			// Calculate how many 16-bit samples we need
			int samples_needed = len / sizeof(int16_t);
			int samples_read = 0;

			if (amic && amic->isActive()) {
				samples_read = amic->readSamples(
					reinterpret_cast<int16_t*>(out),
					samples_needed, 
					timeout_ms);
			}

			if (samples_read == 0) {
				*bytes_read = 0;
				ret = ESP_FAIL;
				break;
			}

			*bytes_read = samples_read * sizeof(int16_t);
			break;
		}
		case MIC_I2S:
			if (imic && imic->isActive()) {
				ret = imic->readAudioData(
					out,
					len,
					bytes_read,
					timeout_ms
				);
				break;
			}

			ret = ESP_FAIL;
			break;
		case MIC_PDM:
			if (pmic && pmic->isActive()) {
				ret = pmic->readAudioData(
					out,
					len,
					bytes_read,
					timeout_ms
				);
				break;
			}

			ret = ESP_FAIL;
			break;
		}

		if (lastSampleLen > 0 && lastSampleLen <= 16000) {
			delete[] lastSample;
			lastSample = nullptr;
			lastSampleLen = 0;
			lastSampleTime = 0;
		}

		if (*bytes_read == 0 || len > 16000) return ret;
		lastSample = (int16_t*) heap_caps_malloc(len, MALLOC_CAP_SPIRAM);
		if (lastSample) {
			memcpy(lastSample, out, len);
			lastSampleLen = len;
			lastSampleTime = millis();
		}

		return ret;
	};

	inline int level() {
		switch (this->micType) {
		case MIC_ANALOG:
			if (amic) {
				return amic->readLevel();
			}
			break;
		case MIC_I2S:
			if (imic) {
				return imic->readLevel();
			}
			break;
		case MIC_PDM:
			if (pmic) {
				return pmic->readLevel();
			}
			break;
		}

		return 0;
	}

	inline Cache getCache() const {
		return Cache{lastSample, lastSampleLen, lastSampleTime};
	}

private:
	MIC_HW micType;
	I2SMicrophone* imic;
	AnalogMicrophone* amic;
	PDMMicrophone* pmic;
	int16_t* lastSample;
	size_t lastSampleLen;
	unsigned long lastSampleTime;
};

extern Microphone* microphone;