/*
 *  _ _ _      _   ___ ___   ______
 * | (_) |__  /_\ | __/ __| / /__  |
 * | | | '_ \/ _ \| _|\__ \/ _ \/ /
 * |_|_|_.__/_/ \_\___|___/\___/_/
 *
 * AES67 real-time audio-over-IP streaming library for C/C++.
 *
 * Copyright (c) 2025 - 2026 Nevio Hirani. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * include/libAES67/endian.h
 * Description: Endian detection and byte-swapping utilities.
 */

#ifndef LIBAES67_ENDIAN_H
#define LIBAES67_ENDIAN_H

#include <stdint.h>

#define LA_LITTLE_ENDIAN 1234
#define LA_BIG_ENDIAN    4321

#if defined(__linux__)
    #include <endian.h>
#elif defined(__APPLE__) || defined(__FreeBSD__)
    #include <sys/endian.h>
#endif

#include "platform.h"

#ifndef LA_BYTE_ORDER
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #define LA_BYTE_ORDER LA_LITTLE_ENDIAN
    #else
        #define LA_BYTE_ORDER LA_BIG_ENDIAN
    #endif
#else
    #define LA_BYTE_ORDER_RUNTIME 1
#endif
#endif

LIBAES67_EXTERN_C_BEGIN

// static inline uint16_t la_htobe16(uint16_t x) { return htobe16(x); }
// static inline uint16_t la_htole16(const uint16_t x) { return htole16(x); }
// static inline uint16_t la_be16toh(uint16_t x) { return be16toh(x); }
// static inline uint16_t la_le16toh(const uint16_t x) { return le16toh(x); }
//
// static inline uint32_t la_htobe32(uint32_t x) { return htobe32(x); }
// static inline uint32_t la_htole32(const uint32_t x) { return htole32(x); }
// static inline uint32_t la_be32toh(uint32_t x) { return be32toh(x); }
// static inline uint32_t la_le32toh(const uint32_t x) { return le32toh(x); }
//
// static inline uint64_t la_htobe64(uint64_t x) { return htobe64(x); }
// static inline uint64_t la_htole64(const uint64_t x) { return htole64(x); }
// static inline uint64_t la_be64toh(uint64_t x) { return be64toh(x); }
// static inline uint64_t la_le64toh(const uint64_t x) { return le64toh(x); }

/*
 * Prefer compiler built-ins for byte-swapping whenever available to
 * improve performance. Falls back to portable implementation if builtin
 * is not supported by the compiler.
 *
 * See: https://gcc.gnu.org/onlinedocs/gcc/Byte-Swapping-Builtins.html
 *      https://man7.org/linux/man-pages/man3/bswap.3.html
 */

#if LA_HAVE_BUILTIN_BSWAP
    #define la_swap16(x) __builtin_bswap16(x)
    #define la_swap32(x) __builtin_bswap32(x)
    #define la_swap64(x) __builtin_bswap64(x)
#else
LA_INLINE uint16_t la_swap16(const uint16_t x) { return (x << 8) | (x >> 8); }
LA_INLINE uint32_t la_swap32(const uint32_t x) {
    return ((x << 24) & 0xff000000) |
           ((x << 8) & 0x00ff0000)  |
           ((x >> 8) & 0x0000ff00)  |
           ((x >> 24) & 0x000000ff);
}
LA_INLINE uint64_t la_swap64(const uint64_t x) {
    const uint32_t hi = (uint32_t)(x >> 32);
    const uint32_t lo = (uint32_t)(x & 0xFFFFFFFF);

    return ((uint64_t)la_swap32(lo) << 32) | la_swap32(hi);
}
#endif

#if LA_BYTE_ORDER == LA_LITTLE_ENDIAN
    #define la_htobe16(x) la_swap16(x)
    #define la_htole16(x) (x)
    #define la_be16toh(x) la_swap16(x)
    #define la_le16toh(x) (x)

    #define la_htobe32(x) la_swap32(x)
    #define la_htole32(x) (x)
    #define la_be32toh(x) la_swap32(x)
    #define la_le32toh(x) (x)

    #define la_htobe64(x) la_swap64(x)
    #define la_htole64(x) (x)
    #define la_be64toh(x) la_swap64(x)
    #define la_le64toh(x) (x)
#else
    #define la_htobe16(x) (x)
    #define la_htole16(x) la_swap16(x)
    #define la_be16toh(x) (x)
    #define la_le16toh(x) la_swap16(x)

    #define la_htobe32(x) (x)
    #define la_htole32(x) la_swap32(x)
    #define la_be32toh(x) (x)
    #define la_le32toh(x) la_swap32(x)

    #define la_htobe64(x) (x)
    #define la_htole64(x) la_swap64(x)
    #define la_be64toh(x) (x)
    #define la_le64toh(x) la_swap64(x)
#endif

LA_INLINE int la_is_little_endian_host(void) {
#ifdef LA_BYTE_ORDER_RUNTIME
    uint16_t x = 1;
    return *((uint8_t *)&x) == 1;
#else
    return LA_BYTE_ORDER == LA_LITTLE_ENDIAN;
#endif
}

LIBAES67_EXTERN_C_END

#endif /* LIBAES67_ENDIAN_H */
