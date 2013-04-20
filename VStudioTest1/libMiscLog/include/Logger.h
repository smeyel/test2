#ifndef __LOGGER_H
#define __LOGGER_H

#include <stdio.h>
#include <stdarg.h>

namespace Logging
{
	/** Base class for logging
	 *  Usage:
	 *  - have an inherited class which implements the vlog method
	 *  - register an instance of the inherited class with registerLogger
	 *  - call log method for logging
	 *
	 */
	class Logger
	{
	protected:
		static Logger* instance;
		static int logLevel;

		virtual ~Logger ();
	public:

		const static int LOGLEVEL_ERROR = 6;
		const static int LOGLEVEL_WARNING = 5;
		const static int LOGLEVEL_INFO = 4;
		const static int LOGLEVEL_DEBUG = 3;
		const static int LOGLEVEL_VERBOSE = 2;


		static void setLogLevel(int _logLevel);

		static int getLogLevel();


		/** Variant of log that takes not a variable argument list but a single va_list pointer.
		 *  Logging should be implemented here, and log method is a simple wrapper around vlog */
		virtual void vlog(int _logLevel, const char *tag, const char *format, va_list argp) = 0;

		static void log(int _logLevel, const char *tag, const char *format, ...);

		static void registerLogger(Logger& _logger);

//		static Logger *getInstance(void);
	};

	class FileLogger : public Logger
	{
		FILE *F;
	public:
		FileLogger(char *filename);

		void close();

		virtual void vlog(int _logLevel, const char *tag, const char *format, va_list argp);

		virtual ~FileLogger();
	};

	class StdoutLogger : public Logger
	{
	public:
		virtual void vlog(int _logLevel, const char *tag, const char *format, va_list argp);
	};
}


#endif
