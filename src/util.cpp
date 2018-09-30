#include "util.h"

namespace util {
namespace string {


std::string join(const std::vector < std::string> &tokens, const std::string delim)
{
    std::string result;
    if (!tokens.empty()) {
        result.append(tokens[0]);
    }
    for (size_t i = 1; i < tokens.size(); ++i) {
        result.append(delim);
        result.append(tokens[i]);
    }
    return result;
}

size_t encode_utf8(char32 c, char *output)
{
    if (c <= 0x7F) {
        *output = static_cast<char>(c);
        return 1;
    }

    if (c <= 0x7FF) {
        output[1] = 0x80 | (c & 0x3F);
        c >>= 6;
        output[0] = 0xC0 | c;
        return 2;
    }

    // if `c` is out-of-range, convert it to REPLACEMENT CHARACTER (U+FFFD).
    // This treatment is the same as the original runetochar.
    if (c > 0x10FFFF) c = kUnicodeError;

    if (c <= 0xFFFF) {
        output[2] = 0x80 | (c & 0x3F);
        c >>= 6;
        output[1] = 0x80 | (c & 0x3F);
        c >>= 6;
        output[0] = 0xE0 | c;
        return 3;
    }

    output[3] = 0x80 | (c & 0x3F);
    c >>= 6;
    output[2] = 0x80 | (c & 0x3F);
    c >>= 6;
    output[1] = 0x80 | (c & 0x3F);
    c >>= 6;
    output[0] = 0xF0 | c;

    return 4;
}

char32 decode_utf8(const char *begin, const char *end, size_t *mblen)
{
    const size_t len = end - begin;

    if (static_cast<unsigned char>(begin[0]) < 0x80) {
        *mblen = 1;
        return static_cast<unsigned char>(begin[0]);
    } else if (len >= 2 && (begin[0] & 0xE0) == 0xC0) {
        const char32 cp = (((begin[0] & 0x1F) << 6) | ((begin[1] & 0x3F)));
        if (is_trail_byte(begin[1]) && cp >= 0x0080 && is_valid_codepoint(cp)) {
            *mblen = 2;
            return cp;
        }
    } else if (len >= 3 && (begin[0] & 0xF0) == 0xE0) {
        const char32 cp = (((begin[0] & 0x0F) << 12) | ((begin[1] & 0x3F) << 6) |
            ((begin[2] & 0x3F)));
        if (is_trail_byte(begin[1]) && is_trail_byte(begin[2]) && cp >= 0x0800 &&
            is_valid_codepoint(cp)) {
            *mblen = 3;
            return cp;
        }
    } else if (len >= 4 && (begin[0] & 0xf8) == 0xF0) {
        const char32 cp = (((begin[0] & 0x07) << 18) | ((begin[1] & 0x3F) << 12) |
            ((begin[2] & 0x3F) << 6) | ((begin[3] & 0x3F)));
        if (is_trail_byte(begin[1]) && is_trail_byte(begin[2]) &&
            is_trail_byte(begin[3]) && cp >= 0x10000 && is_valid_codepoint(cp)) {
            *mblen = 4;
            return cp;
        }
    }

    // Invalid UTF-8.
    *mblen = 1;
    return kUnicodeError;
}

std::u32string utf8text_to_unicodetext(std::string utf8_text)
{
    std::u32string uc;
    const char *begin = utf8_text.c_str();
    const char *end = utf8_text.c_str() + utf8_text.length();
    while (begin < end) {
        size_t mblen;
        const char32 c = decode_utf8(begin, end, &mblen);
        uc.push_back(c);
        begin += mblen;
    }
    return uc;
}

std::string unicodetext_to_utf8text(std::u32string unicode_text)
{
    char buf[8];
    std::string result;
    for (const char32 c : unicode_text) {
        const size_t mblen = encode_utf8(c, buf);
        result.append(buf, mblen);
    }
    return result;
}

} // namespace string
} // namespace util