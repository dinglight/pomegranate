#ifndef __LOG_H__
#define __LOG_H__
#include <stdio.h>

#define LOG(severity, ...)                               \
    do {                                                 \
        fprintf(stderr, "(%s)", #severity);              \
        fprintf(stderr, "%s(%d):\t", __FILE__, __LINE__);\
        fprintf(stderr, __VA_ARGS__);                    \
        fprintf(stderr, "\n");                           \
    } while(0)

#endif /* __LOG_H__ */
