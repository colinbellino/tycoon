#if !defined(UTILS_H)

#include <stdio.h>
#include <time.h>

#define kilobytes(value) ((value)*1024LL)
#define megabytes(value) (kilobytes(value) * 1024LL)
#define gigabytes(value) (megabytes(value) * 1024LL)
#define terabytes(value) (gigabytes(Value) * 1024LL)

#define arrayCount(array) (sizeof(array) / sizeof((array)[0]))

#define print(...)       \
    printf(__VA_ARGS__); \
    fflush(stdout);

#define assert(expression, message)                                                    \
    if ((expression) == 0)                                                             \
    {                                                                                  \
        printf("Assertion failed in %s, line %d: %s.\n", __FILE__, __LINE__, message); \
    }

void printTime(const char *prefix, time_t time)
{
    char date[18];
    strftime(date, 18, "%d-%m-%y %H:%M:%S", gmtime(&time));
    printf("%s%s\n", prefix, date);
    date[0] = 0;
}

#define UTILS_H
#endif
