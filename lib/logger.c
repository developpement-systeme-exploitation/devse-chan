#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <libgen.h>
#include "logger.h"

static struct logger logger = {
#ifdef NDEBUG
	LOG_INFO,
#else
	LOG_DEBUG,
#endif /* NDEBUG */
	NULL,
};

static const char *const LEVEL_STR[] = {
	"ERROR",
	"WARN ",
	"INFO ",
	"TRACE",
	"DEBUG"
};

static const char *const LEVEL_COLOR[] = {
	"\033[31m",
	"\033[33m",
	"\033[34m",
	"",
	""
};

void
logger_init_default(void)
{
	logger.out = stdout;
}

static void
logger_log_meta(enum logger_level level, const char *file, int line)
{
	time_t t;
	struct tm tm;
	char buffer[128];
	const char *bname;

	if (logger.out == NULL)
	{
		logger.out = stdout;
	}

	if (level == LOG_DEBUG)
	{
		fprintf(logger.out, "\033[90m");
	}

	t = time(NULL);
	localtime_s(&tm, &t);
	strftime(buffer, 128, "%x %X", &tm);

	bname = basename((char *)file);
	fprintf(logger.out, "%s %10s:%d: [\033[1;1m%s%s", buffer, bname,
		line, LEVEL_COLOR[level], LEVEL_STR[level]);
	if (level != LOG_DEBUG)
	{
		fprintf(logger.out, "\033[0m");
	}

	fprintf(logger.out, "]: ");
}

void
logger_log(enum logger_level level, const char *file, int line,
			const char *msg, ...)
{
	va_list arg;

	if (level > logger.level)
	{
		return;
	}

	va_start(arg, msg);

	logger_log_meta(level, file, line);
	
	vfprintf(logger.out, msg, arg);

	fprintf(logger.out, "\n");

	if (level == LOG_DEBUG)
	{
		fprintf(logger.out, "\033[0m");
	}

	va_end(arg);
}

void
logger_log_hexdump(enum logger_level level, const char *file, int line,
			const uint8_t *data, size_t size)
{
	size_t i;
	size_t j;
	uint32_t addr;

	logger_log_meta(level, file, line);

	fprintf(logger.out, "\n");
	addr = 0;
	for (i = 0; i < size; i++)
	{
		if (i == 0 || i % 16 == 0)
		{
			fprintf(logger.out, "\n\t%08X\t", addr);
		}
		else if (i % 8 == 0)
		{
			fprintf(logger.out, " ");
		}
		
		fprintf(logger.out, "%02X ", data[i]);
		addr++;
	}

	if (level == LOG_DEBUG)
	{
		fprintf(logger.out, "\033[0m");
	}

	
	fprintf(logger.out, "\n\n");
}