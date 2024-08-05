#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <sstream>

namespace Covlog
{
enum Type
{
	eError,
	eWarning,
	eDebug,
	eInfo,
	eCount
};

void LogMessage(Type type, const char *fileName, int lineNumber, const char *message);

template<typename... Args>
void Log(Covlog::Type type, const char *fileName, int lineNumber, Args &&...args);
}// namespace Covlog

template<typename... Args>
void Covlog::Log(Covlog::Type type, const char *fileName, int lineNumber, Args &&...args)
{
	// FIXME: I hate this function

	std::ostringstream os;
	(os << ... << std::forward<Args>(args));
	LogMessage(type, fileName, lineNumber, os.str().c_str());
}

#ifndef __FILE_NAME__
#define __FILE_NAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#endif

#define CLOG(logger, ...) Covlog::Log(logger, __FILE_NAME__, __LINE__, "", __VA_ARGS__)
#define CLOG_ERR(...) CLOG(Covlog::eError, __VA_ARGS__)
#define CLOG_WARN(...) CLOG(Covlog::eWarning, __VA_ARGS__)
#define CLOG_DEBUG(...) CLOG(Covlog::eDebug, __VA_ARGS__)
#define CLOG_INFO(...) CLOG(Covlog::eInfo, __VA_ARGS__)

#define COV_ASSERT(expr, msg)          \
	if (expr)                          \
	{                                  \
	}                                  \
	else                               \
	{                                  \
		CLOG_ERR((msg));               \
		assert(((void)(msg), (expr))); \
	}

#endif//OGL_0_LOGGER_HPP
