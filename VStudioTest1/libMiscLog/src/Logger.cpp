#include <stdio.h>
#include <stdarg.h>
#include "Logger.h"

namespace Logging
{

	void Logger::setLogLevel(int _logLevel)
	{
		logLevel = _logLevel;
	}

	int Logger::getLogLevel()
	{
		return logLevel;
	}

	void Logger::log(int _logLevel, const char *tag, const char *format, ...)
	{
		va_list argp;
		va_start (argp, format);

		if(instance != NULL)
		{
			instance->vlog(_logLevel, tag, format, argp);
		}

		va_end (argp);
	}

	void Logger::registerLogger(Logger& _logger) {
		instance = &_logger;
	}

//	Logger* Logger::getInstance() {
//		return instance;
//	}

	Logger::~Logger() {}


	Logger *Logger::instance = NULL;
	int Logger::logLevel = Logger::LOGLEVEL_WARNING;





	FileLogger::FileLogger(char *filename)
	{
		F = fopen(filename,"at");
	}
	void FileLogger::close()
	{
		fflush(F);
		fclose(F);
	}

	void FileLogger::vlog(int _logLevel, const char *tag, const char *format, va_list argp)
	{
		if (_logLevel >= logLevel)
		{
			vfprintf (F, format, argp);
		}
	}

	FileLogger::~FileLogger() {
		this->close();
	}

	void StdoutLogger::vlog(int _logLevel, const char *tag, const char *format, va_list argp)
	{
		if (_logLevel >= logLevel)
		{
			vfprintf (stdout, format, argp);
		}
	}
}


//int main()
//{
//	using namespace Logging;
//	int i = 12;
//
//	FileLogger loggerF("d:\\e3.txt");
//	//StdoutLogger loggerStd;
//
//	Logger::getInstance()->log(Logger::LOGLEVEL_ERROR,"TAG","Szam:%d %d %s %d\n",1,2,"Hello",3);
//
//	loggerF.close();
//	return 0;
//}
