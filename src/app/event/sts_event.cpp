#include <app/events.h>

void stsTools(){
	aiSts.addTool(GPTStsService::GPTTool{
		.description = "System weather information, cannot be changed",
		.name = "weather"
	});

	GPTSpiJsonDocument timeParam;
	timeParam["type"] = "object";
	// https://platform.openai.com/docs/api-reference/realtime-client-events/session/update
	timeParam["properties"]["type"]["type"] = "string";
	timeParam["properties"]["type"]["description"] = "a time type. value is 'date', 'time', 'datetime'";
	timeParam.shrinkToFit();
	aiSts.addTool(GPTStsService::GPTTool{
		.description = "System time",
		.name = "time",
		.params = timeParam
	});
	aiSts.addTool(GPTStsService::GPTTool{
		.description = "Close conversation talk",
		.name = "end_conversation_session"
	});
	aiSts.addTool(GPTStsService::GPTTool{
		.description = "Restart the system",
		.name = "restart"
	});

	aiSts.sendTools();
	notification->send(NOTIFICATION_DISPLAY, EDISPLAY_FACE);
	aiSts.Speak();
}

void stsEvent(const GPTStsService::GPTToolCall& data) {
	ESP_LOGI("AIFunctionCall", "name: %s, call_id: %s, params: %s", data.name, data.callId, data.params.as<String>().c_str());
	// need move to command processor
	if (0 == strcmp(data.name, "weather"))
		weatherService.getCurrentWeather([&data](weatherData_t wdata, bool success){
			String resp = 
				"Temperature: " + String(wdata.temperature)
				+". Humidity: " + String(wdata.humidity)
				+". Wind Speed: " + String(wdata.windSpeed)
				+". Wind Direction: " + String(wdata.windDirection)
				+". Deskripsi: " + String(wdata.description)
				+". Last Update: " + String(wdata.lastUpdated);
			
			aiSts.sendToolCallback(GPTStsService::GPTToolCallback{
				.callId = data.callId,
				.name = data.name,
				.output = resp.c_str(),
				.status = "complete"
			});
		});
	else if (0 == strcmp(data.name, "time"))
		aiSts.sendToolCallback(GPTStsService::GPTToolCallback{
			.callId = data.callId,
			.name = data.name,
			.output = timeManager.getCurrentTime(),
			.status = "complete"		
		});
	else if (0 == strcmp(data.name, "end_conversation_session")){
		notification->send(NOTIFICATION_DISPLAY, EDISPLAY_NONE);
		aiSts.stop();
		delay(10);
		speaker->clear();
	}
	else if (0 == strcmp(data.name, "restart"))
		ESP.restart();

	if (!data.params.isNull()) data.params;
}

void srDisconnectCallback() {
	notification->send(NOTIFICATION_DISPLAY, EDISPLAY_NONE);
}