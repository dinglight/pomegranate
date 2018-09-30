#ifndef __UTIL_H__
#define __UTIL_H__
#include <string>
#include <vector>
#include "common.h"

namespace util {

namespace string {

// Return (x & 0xC0) == 0x80;
// Since trail bytes are always in [0x80, 0xBF], we can optimize:
inline bool is_trail_byte(char x)
{
    return static_cast<signed char>(x) < -0x40;
}

inline bool is_valid_codepoint(char32 c)
{
    return (static_cast<uint32>(c) < 0xD800) || (c >= 0xE000 && c <= 0x10FFFF);
}

std::string join(const std::vector < std::string> &tokens, const std::string delim);

std::u32string utf8text_to_unicodetext(std::string utf8_text);
std::string unicodetext_to_utf8text(std::u32string unicode_text);
} // namespace string


namespace stl {

template <typename T>
void delete_vector_elements(std::vector<T *> *vec)
{
    for (auto item : *vec) {
        delete item;
    }
    vec->clear();
}

} // namespace stl
} // namespace util
#endif // !__UTIL_H__
