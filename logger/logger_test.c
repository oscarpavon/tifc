#include "logger.h"

static logger_t Logger;

int main(void)
{
    logger_init(&Logger, LOGGER_INFO, "./my_log.log");

    (void) logger_log(&Logger, LOGGER_INFO,
        "Hello from logger: %d %#x %s \n", 255, 255, "string");

    (void) logger_log(&Logger, LOGGER_CRITICAL,
        "ERROR:: from logger: %d %#x %s \n", 255, 255, "string");

    logger_deinit(&Logger);
    return 0;
}


