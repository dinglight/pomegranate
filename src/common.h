#ifndef __COMMON_H__
#define __COMMON_H__

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t char32;
typedef uint32_t uint32;
typedef uint64_t uint64;

static constexpr uint32 kUnicodeNone = 0x0000;
static constexpr uint32 kUnicodeSpace = 0x0020;
static constexpr uint32 kUnicodeError = 0xFFFD;

#define RETURN_IF_FALSE(expr)     \
    do {                          \
        bool ok = expr;           \
        if (!ok) return ok;       \
    } while(0)




#endif // !__COMMON_H__

