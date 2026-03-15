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
 * include/libAES67/platform.h
 * Description: Platform and compiler abstraction macros.
 */

#ifndef LIBAES67_PLATFORM_H
#define LIBAES67_PLATFORM_H

#include <stdarg.h>
#include <_stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
    #define __LA_BEGIN_C_DECLS extern "C" {
    #define __LA_END_C_DECLS   }
#else
    #define __LA_BEGIN_C_DECLS
    #define __LA_END_C_DECLS
#endif

#ifdef __cplusplus
#define __LA_EXTERN_C extern "C"
#else
#define __LA_EXTERN_C
#endif

#if defined(_WIN32) || defined(_WIN64)
#error "Windows is not supported by libAES67."
#endif

#ifdef _MSC_VER
#error "MSVC is not supported by libAES67."
#endif

#if !defined(__clang__) && !defined(__GNUC__)
#error "Unsupported compiler. libAES67 only supports GCC and Clang."
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define LA_HAVE_BUILTIN_BSWAP 1
#else
    #define LA_HAVE_BUILTIN_BSWAP 0
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define LA_LIKELY(x)   __builtin_expect(!!(x), 1)
    #define LA_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    #define LA_LIKELY(x)   (x)
    #define LA_UNLIKELY(x) (x)
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define LA_ALIGNED(x) __attribute__((aligned(x)))
#else
    #define LA_ALIGNED(x)
#endif

#ifndef LA_INLINE
    #ifdef __cplusplus
        #define LA_INLINE inline
    #else
        #define LA_INLINE static inline
    #endif
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define LA_DEPRECATED(msg) __attribute__((deprecated(msg)))
#else
    #define LA_DEPRECATED(msg)
#endif

#if defined(_WIN32)
    #define LA_API __declspec(dllexport)
#else
    #define LA_API __attribute__((visibility("default")))
#endif

__LA_BEGIN_C_DECLS

#define xasprintf(strp, ...) la_xasprintf((strp), ##__VA_ARGS__)

LA_INLINE int la_xasprintf(char **strp, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

#if defined(__GLIBC__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    const int ret = vasprintf(strp, fmt, args);
    va_end(args);
    return ret;
#else
    va_list args_copy;
    va_copy(args_copy, args);

    int len = vsnprintf(NULL, 0, fmt, args);
    if (len < 0) {
        va_end(args_copy);
        va_end(args);
        return -1;
    }

    *strp = malloc(len + 1);
    if (!*strp) {
        va_end(args_copy);
        va_end(args);
        return -1;
    }

    int ret = vsnprintf(*strp, (size_t)len + 1, fmt, args_copy);
    if (ret < 0) {
        free(*strp);
        *strp = NULL;
    }

    va_end(args_copy);
    va_end(args);

    return ret;
#endif
}

__LA_END_C_DECLS

#endif /* LIBAES67_PLATFORM_H */
