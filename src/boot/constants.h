#ifndef BOOT_CONSTANTS_H
#define BOOT_CONSTANTS_H

#include "csr.h"

#define SR_CMD_STR_LEN_MAX     64
#define SR_CMD_PHONEME_LEN_MAX 64

// Voice command IDs
enum CommandId {
    CMD_WAKEUP = 0,
    CMD_TIME,
    CMD_WEATHER,
    CMD_RECORD_AUDIO,
    CMD_MAX
};

// Define voice commands (phonetic representations)
static const SR::csr_cmd_t voice_commands[] = {
	{CMD_WAKEUP, "Hi Cozmo",    "hi KbZMb"},
	{CMD_TIME, "time",    "TiM"},
	{CMD_WEATHER, "weather", "Wfjk"},
	{CMD_RECORD_AUDIO, "record audio", "RfKkD nDmb"}
};

static const char* NOTIFICATION_DISPLAY = "display";
static const char* NOTIFICATION_WEATHER = "weather";
static const char* NOTIFICATION_COMMAND = "command";

// Display Events
static const char* EVENT_DISPLAY_WAKEWORD = "display_wakeword";

#endif