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
 * @file include/libAES67/__tai.h
 * @brief UTC leap second transition timestamps.
 *
 * Data source:
 * IERS Bulletin C / NTP leap-seconds.list
 */

#ifndef LIBAES67___TAI_H
#define LIBAES67___TAI_H

#ifndef LIBAES67_INTERNAL_INCLUDE
#error "Do not include this header directly. Use <libAES67/time.h>."
#endif

#include <libAES67/platform.h>

#define LA_TAI_UTC_INITIAL_OFFSET        10
#define LA_EXPECTED_FINAL_TAI_UTC_OFFSET 37
#define LA_NS_PER_SEC                    1000000000LL

__LA_BEGIN_C_DECLS

/**
 * @brief UTC leap second transition table.
 *
 * Unix epoch timestamps (UTC) at which the TAI−UTC offset increased
 * due to an inserted leap second.
 *
 * Initial offset on 1972-01-01 is 10 seconds.
 */
static const int64_t __la_leap_seconds_utc[] = {
    78796800,     /* 1972-07-01 */
    94694400,     /* 1973-01-01 */
    126230400,    /* 1974-01-01 */
    157766400,    /* 1975-01-01 */
    189302400,    /* 1976-01-01 */
    220924800,    /* 1977-01-01 */
    252460800,    /* 1978-01-01 */
    283996800,    /* 1979-01-01 */
    315532800,    /* 1980-01-01 */
    362793600,    /* 1981-07-01 */
    394329600,    /* 1982-07-01 */
    425865600,    /* 1983-07-01 */
    489024000,    /* 1985-07-01 */
    567993600,    /* 1988-01-01 */
    631152000,    /* 1990-01-01 */
    662688000,    /* 1991-01-01 */
    709948800,    /* 1992-07-01 */
    741484800,    /* 1993-07-01 */
    773020800,    /* 1994-07-01 */
    820454400,    /* 1996-01-01 */
    867715200,    /* 1997-07-01 */
    915148800,    /* 1999-01-01 */
    1136073600,   /* 2006-01-01 */
    1230768000,   /* 2009-01-01 */
    1341100800,   /* 2012-07-01 */
    1435708800,   /* 2015-07-01 */
    1483228800    /* 2017-01-01 */
};

#define LA_LEAP_TABLE_COUNT \
    (sizeof(__la_leap_seconds_utc) / sizeof(__la_leap_seconds_utc[0]))

_Static_assert(LA_LEAP_TABLE_COUNT > 0, "Leap second table must not be empty");
_Static_assert((LA_TAI_UTC_INITIAL_OFFSET + LA_LEAP_TABLE_COUNT) == LA_EXPECTED_FINAL_TAI_UTC_OFFSET,
               "Final TAI-UTC offset mismatch");

#define LA_TAI_UTC_OFFSET \
    (LA_TAI_UTC_INITIAL_OFFSET + LA_LEAP_TABLE_COUNT)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-identifier"

/**
 * @brief Count number of leap seconds that have occurred up to a UTC timestamp.
 *
 * Complexity: O(log n)
 *
 * @param utc_sec UTC timestamp in seconds.
 * @return Number of leap seconds to add.
 */
LA_INLINE size_t __la_leap_count(const int64_t utc_sec) {
    size_t lhs = 0;
    size_t rhs = LA_LEAP_TABLE_COUNT;

    while (lhs < rhs) {
        const size_t mid = lhs + (( rhs - lhs ) / 2);
        if (utc_sec >= __la_leap_seconds_utc[mid]) {
            lhs = mid + 1;
        } else {
            rhs = mid;
        }
    }

    return lhs;
}

/**
 * @brief Compute the TAI offset for a given UTC timestamp.
 *
 * @param utc_sec UTC timestamp in seconds.
 * @return TAI−UTC offset in seconds.
 */
LA_INLINE int64_t __la_tai_offset(const int64_t utc_sec) {
    return LA_TAI_UTC_INITIAL_OFFSET + (int64_t)__la_leap_count(utc_sec);
}

#pragma clang diagnostic pop

/**
 * @brief Convert UTC seconds to TAI seconds.
 *
 * @param utc_sec UTC timestamp in seconds.
 * @return Corresponding TAI timestamp in seconds.
 */
LA_INLINE int64_t la_utc_to_tai(const int64_t utc_sec) {
    return utc_sec + __la_tai_offset(utc_sec);
}

/**
 * @brief Convert TAI seconds to UTC seconds.
 *
 * @param tai_sec TAI timestamp in seconds.
 * @return Corresponding UTC timestamp in seconds.
 */
LA_INLINE int64_t la_tai_to_utc(const int64_t tai_sec) {
    int64_t utc_sec = tai_sec - LA_TAI_UTC_INITIAL_OFFSET;
    utc_sec -= (int64_t)__la_leap_count(utc_sec);

    return utc_sec;
}


/**
 * @brief Convert UTC nanoseconds to TAI nanoseconds.
 *
 * @param utc_ns UTC timestamp in nanoseconds.
 * @return Corresponding TAI timestamp in nanoseconds.
 */
LA_INLINE int64_t la_utc_ns_to_tai_ns(const int64_t utc_ns) {
    const int64_t utc_sec = utc_ns / LA_NS_PER_SEC;
    const int64_t offset = __la_tai_offset(utc_sec);

    return utc_ns + (offset * LA_NS_PER_SEC);
}

/**
 * @brief Convert TAI nanoseconds to UTC nanoseconds.
 *
 * @param tai_ns TAI timestamp in nanoseconds.
 * @return Corresponding UTC timestamp in nanoseconds.
 */
LA_INLINE int64_t la_tai_ns_to_utc_ns(const int64_t tai_ns) {
    const int64_t tai_sec = tai_ns / LA_NS_PER_SEC;
    int64_t utc_sec = tai_sec - LA_TAI_UTC_INITIAL_OFFSET;
    utc_sec -= (int64_t)__la_leap_count(utc_sec);

    return tai_ns - ((tai_sec - utc_sec) * LA_NS_PER_SEC);
}

__LA_END_C_DECLS

#endif /* LIBAES67___TAI_H */
