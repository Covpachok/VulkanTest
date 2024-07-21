#include "logger.h"

#include <cstdio>

#ifdef LOGGER_NO_ANSI_COLORS
#define COLOR_ESCAPE_START(Color) ""
#define COLOR_ESCAPE_END() ""
#else
#define COLOR_ESCAPE_START(Color) "\033[" #Color ";1m"
#define COLOR_ESCAPE_END() "\033[0m"
#endif

#define COLORIZED_TEXT(Text, Color) COLOR_ESCAPE_START(Color) Text COLOR_ESCAPE_END()

constexpr const char *kPrefixes[]{
		COLORIZED_TEXT("[ERROR]  ", 31),
		COLORIZED_TEXT("[WARNING]", 33),
		COLORIZED_TEXT("[DEBUG]  ", 36),
		COLORIZED_TEXT("[INFO]   ", 32)
};

constexpr const char *kPrefixesColors[]{
		COLOR_ESCAPE_START(31), COLOR_ESCAPE_START(33), COLOR_ESCAPE_START(36), COLOR_ESCAPE_START(32)
};

constexpr size_t kLogBufferSize = 1024;

void Covlog::LogMessage(Covlog::Type type, const char *fileName, int lineNumber, const char *message)
{
	time_t now{time(nullptr)};
	tm     time{};
	localtime_s(&time, &now);

	char logBuffer[kLogBufferSize]{};

	snprintf(
			logBuffer,
			kLogBufferSize,
			"%02i:%02i:%02i %s %s%s:%i%s: %s%s%s\n",
			time.tm_hour,
			time.tm_min,
			time.tm_sec,
			kPrefixes[type],
			COLOR_ESCAPE_START(34),
			fileName,
			lineNumber,
			COLOR_ESCAPE_END(),
			kPrefixesColors[type],
			message,
			COLOR_ESCAPE_END()
	);

	fwrite(logBuffer, kLogBufferSize, 1, stdout);
}
