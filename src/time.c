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
 * src/time.c
 * Description: Clock and time abstraction for PTP.
 */

#include <time.h>
#include <errno.h>
#include <_string.h>

#include "../include/libAES67/time.h"

LA_INLINE void la_normalize_timespec(struct timespec *ts) {
    ts->tv_sec += ts->tv_nsec / LA_NS_PER_SEC;
    ts->tv_nsec %= LA_NS_PER_SEC;
}

LA_INLINE bool la_clock_is_relative(const la_clock_t clock_type) {
    switch (clock_type) {
        case LA_CLOCK_MONOTONIC:
        case LA_CLOCK_PROCESS:
        case LA_CLOCK_THREAD:
            return true;
        default:
            return false;
    }
}

int la_time_get(la_time_t *xtp, const la_clock_t clock_type) {
    if (!xtp) { return -1; }

    struct timespec ts;

    switch (clock_type) {
        case LA_CLOCK_UTC:
            if (clock_gettime(CLOCK_REALTIME, &ts) != 0) { return -1; }
            break;

        case LA_CLOCK_TAI:
            #ifdef CLOCK_TAI
                if (clock_gettime(CLOCK_TAI, &ts) != 0) { return -1; }
            #else
                /*
                 * Simulate TAI on platforms without CLOCK_TAI (e.g., macOS).
                 * Read UTC via CLOCK_REALTIME and compute TAI using leap second table <__tai.h>.
                 */
                if (clock_gettime(CLOCK_REALTIME, &ts) != 0) { return -1; }

                int64_t ts_ns = ((int64_t)ts.tv_sec * LA_NS_PER_SEC) + ts.tv_nsec;

                ts_ns = la_utc_ns_to_tai_ns(ts_ns);

                ts.tv_sec  = ts_ns / LA_NS_PER_SEC;
                ts.tv_nsec = (long)(ts_ns % LA_NS_PER_SEC);
            #endif
            break;

        case LA_CLOCK_MONOTONIC:
            #ifdef CLOCK_MONOTONIC_RAW
                if (clock_gettime(CLOCK_MONOTONIC_RAW, &ts) != 0) { return -1; }
            #else
                if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) { return -1; }
            #endif
            break;

        case LA_CLOCK_PROCESS:
            if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts) != 0) { return -1; }
            break;

        case LA_CLOCK_THREAD:
            if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts) != 0) { return -1; }
            break;

        case LA_CLOCK_PTP:
        case LA_CLOCK_PTPv2:
            // TODO: Implement PTP via ptp.h
            return -1;

        default:
            return -1;
    }

    xtp->sec = ts.tv_sec;
    xtp->nsec = (int_fast32_t)ts.tv_nsec;
    la_time_normalize(xtp);

    return 0;
}

int la_time_getres(la_time_t *res, const la_clock_t clock_type) {
    if (!res) { return -1; }

    struct timespec ts;

    switch (clock_type) {
        case LA_CLOCK_UTC:
            if (clock_getres(CLOCK_REALTIME, &ts) != 0) { return -1; }
            break;

        case LA_CLOCK_TAI:
            #ifdef CLOCK_TAI
                if (clock_getres(CLOCK_TAI, &ts)  != 0) { return -1; }
            #else
                /*
                 * Simulate TAI resolution on platforms without CLOCK_TAI (e.g., macOS).
                 *
                 * We query the resolution of CLOCK_REALTIME (UTC) and convert the interval
                 * into the equivalent TAI interval using the UTC↔TAI conversion functions
                 * from <__tai.h>.
                 *
                 * Since TAI differs from UTC only by an integer number of leap seconds,
                 * the actual clock resolution is identical. However, converting through
                 * la_utc_ns_to_tai_ns() ensures the returned interval is expressed in
                 * the same TAI timescale used by the rest of the library.
                 *
                 * The subtraction of la_utc_ns_to_tai_ns(0) removes the constant epoch
                 * offset (TAI-UTC = 10 s at 1970-01-01), leaving only the interval size.
                 */
                if (clock_getres(CLOCK_REALTIME, &ts) != 0) { return -1; }

                int64_t res_ns = ((int64_t)ts.tv_sec * LA_NS_PER_SEC) + ts.tv_nsec;

                res_ns = la_utc_ns_to_tai_ns(res_ns) - la_utc_ns_to_tai_ns(0);
                ts.tv_sec  = res_ns / LA_NS_PER_SEC;
                ts.tv_nsec = (long)(res_ns % LA_NS_PER_SEC);
            #endif
            break;

        case LA_CLOCK_MONOTONIC:
            #ifdef CLOCK_MONOTONIC_RAW
                if (clock_getres(CLOCK_MONOTONIC_RAW, &ts) != 0) { return -1; }
            #else
                if (clock_getres(CLOCK_MONOTONIC, &ts) != 0) { return -1; }
            #endif
            break;

        case LA_CLOCK_PROCESS:
            if (clock_getres(CLOCK_PROCESS_CPUTIME_ID, &ts) != 0) { return -1; }
            break;

        case LA_CLOCK_THREAD:
            if (clock_getres(CLOCK_THREAD_CPUTIME_ID, &ts) != 0) { return -1; }
            break;

        case LA_CLOCK_PTP:
        case LA_CLOCK_PTPv2:
            // TODO: Implement PTP resolution
            return -1;

        default:
            return -1;
    }

    res->sec = ts.tv_sec;
    res->nsec = (int_fast32_t)ts.tv_nsec;

    return 0;
}

int la_time_sleep(const la_time_t *duration) {
    if (!duration) { return -1; }
    if (duration->sec < 0 || (duration->sec == 0 && duration->nsec <= 0)) { return 0; }

    struct timespec req;
    struct timespec rem;

    req.tv_sec  = duration->sec;
    req.tv_nsec = (long)(duration->nsec);

    /* Normalize duration to ensure tv_nsec < 1e9 */
    la_normalize_timespec(&req);

    while (nanosleep(&req, &rem) != 0) {
        if (errno != EINTR) {
            return -1;
        }
        req = rem;
    }

    return 0;
}

int la_time_sleep_until(const la_time_t *target, const la_clock_t clock_type) {
    if (!target) { return -1; }

    la_time_t now;
    if (la_time_get(&now, clock_type) != 0) { return -1; }

    int_fast64_t delta_sec  = target->sec - now.sec;
    int_fast64_t delta_nsec = (int_fast64_t)target->nsec - now.nsec;

    if (delta_nsec < 0) {
        delta_sec  -= 1;
        delta_nsec += LA_NS_PER_SEC;
    }

    /* Return if target is in the past */
    if (delta_sec < 0 || (delta_sec == 0 && delta_nsec <= 0)) {
        return 0;
    }

    struct timespec req;
    struct timespec rem;

    req.tv_sec  = (time_t)delta_sec;
    req.tv_nsec = (long)delta_nsec;

    /* Normalize duration to ensure tv_nsec < 1e9 */
    la_normalize_timespec(&req);

    while (nanosleep(&req, &rem) != 0) {
        if (errno != EINTR) {
            return -1;
        }
        req = rem;
    }

    return 0;
}

int la_time_conv(la_time_t *dst, const la_clock_t dst_clock_type, const la_time_t *src,
                 const la_clock_t src_clock_type) {
    if (!dst || !src) {
        return -1;
    }

    if (dst_clock_type == src_clock_type) {
        *dst = *src;
        return 0;
    }

    /*
     * Relative clocks (MONOTONIC, PROCESS, THREAD) measure time relative to
     * an implementation-defined epoch and do not track wall-clock time.
     *
     * Converting between relative clocks and absolute clocks (UTC, TAI, PTP)
     * would therefore require additional kernel state and cannot be performed
     * reliably here. Such conversions are rejected to avoid producing
     * misleading results.
     */
    if (la_clock_is_relative(src_clock_type) !=
        la_clock_is_relative(dst_clock_type)) {
        return -1;
    }

    int64_t ns;
    if (__builtin_mul_overflow(src->sec, LA_NS_PER_SEC, &ns)) {
        return -1;
    }

    if (__builtin_add_overflow(ns, src->nsec, &ns)) {
        return -1;
    }

    switch (src_clock_type) {
        case LA_CLOCK_UTC:
            /* UTC is used as the baseline; no adjustments required */
            break;

        case LA_CLOCK_TAI:
        case LA_CLOCK_PTP:
        case LA_CLOCK_PTPv2:
            /*
             * As PTP clocks are usually synchronized to TAI internally we can
             * convert them via la_tai_ns_to_utc_ns()
             */
            ns = la_tai_ns_to_utc_ns(ns);
            break;

        case LA_CLOCK_MONOTONIC:
        case LA_CLOCK_PROCESS:
        case LA_CLOCK_THREAD:
            break;

        default:
            return -1;
    }

    switch (dst_clock_type) {
        case LA_CLOCK_UTC:
            /* Already in UTC baseline */
            break;

        case LA_CLOCK_TAI:
        case LA_CLOCK_PTP:
        case LA_CLOCK_PTPv2:
            /*
             * As PTP clocks are usually synchronized to TAI internally we can
             * convert baseline UTC to TAI via la_utc_ns_to_tai_ns()
             */
            ns = la_utc_ns_to_tai_ns(ns);
            break;

        case LA_CLOCK_MONOTONIC:
        case LA_CLOCK_PROCESS:
        case LA_CLOCK_THREAD:
            return -1;

        default:
            return -1;
    }

    dst->sec = ns / LA_NS_PER_SEC;
    dst->nsec = (int_fast32_t)(ns % LA_NS_PER_SEC);
    la_time_normalize(dst);

    return 0;
}

int la_time_normalize(la_time_t *xtp) {
    if (!xtp) {
        return -1;
    }

    xtp->sec += xtp->nsec / LA_NS_PER_SEC;
    xtp->nsec %= LA_NS_PER_SEC;

    if (xtp->nsec < 0) {
        xtp->sec--;
        xtp->nsec += LA_NS_PER_SEC;
    }

    return 0;
}

int la_time_add(la_time_t *dst, const la_time_t *lhs, const la_time_t *rhs) {
    if (!dst || !lhs || !rhs) {
        return -1;
    }

    dst->nsec = lhs->nsec + rhs->nsec;
    dst->sec  = lhs->sec  + rhs->sec;
    dst->min  = lhs->min  + rhs->min;
    dst->hour = lhs->hour + rhs->hour;
    dst->day  = lhs->day  + rhs->day;

    if (dst->nsec >= LA_NS_PER_SEC) {
        dst->sec += dst->nsec / LA_NS_PER_SEC;
        dst->nsec %= LA_NS_PER_SEC;
    }

    if (dst->sec >= LA_SEC_PER_MIN) {
        dst->min += dst->sec / LA_SEC_PER_MIN;
        dst->sec %= LA_SEC_PER_MIN;
    }

    if (dst->min >= LA_MIN_PER_HOUR) {
        dst->hour += dst->min / LA_MIN_PER_HOUR;
        dst->min %= LA_MIN_PER_HOUR;
    }

    if (dst->hour >= LA_HOUR_PER_DAY) {
        dst->day += dst->hour / LA_HOUR_PER_DAY;
        dst->hour %= LA_HOUR_PER_DAY;
    }

    return 0;
}

int la_time_sub(la_time_t *dst, const la_time_t *lhs, const la_time_t *rhs) {
    if (!dst || !lhs || !rhs) {
        return -1;
    }

    /*
     * Ensure the result will not be negative.
     * If rhs > lhs, subtraction cannot proceed.
     */
    if (la_time_cmp(lhs, rhs) < 0) {
        return -1;
    }

    /*
     * Borrow from the next larger unit if a field underflows:
     *
     *   nsec < 0  -> borrow 1 sec
     *   sec  < 0  -> borrow 1 min
     *   min  < 0  -> borrow 1 hour
     *   hour < 0  -> borrow 1 day
     *
     * This ensures all fields remain in their canonical ranges:
     * 0 <= nsec < 1e9, 0 <= sec < 60, 0 <= min < 60, 0 <= hour < 24
     */
    dst->nsec = lhs->nsec - rhs->nsec;
    dst->sec  = lhs->sec  - rhs->sec;
    dst->min  = lhs->min  - rhs->min;
    dst->hour = lhs->hour - rhs->hour;
    dst->day  = lhs->day  - rhs->day;

    if (dst->nsec < 0) {
        dst->sec--;
        dst->nsec += LA_NS_PER_SEC;
    }

    if (dst->sec < 0) {
        dst->min--;
        dst->sec += LA_SEC_PER_MIN;
    }

    if (dst->min < 0) {
        dst->hour--;
        dst->min += LA_MIN_PER_HOUR;
    }

    if (dst->hour < 0) {
        dst->day--;
        dst->hour += LA_HOUR_PER_DAY;
    }

    return 0;
}

int la_time_cmp(const la_time_t *lhs, const la_time_t *rhs) {
    if (!lhs || !rhs) {
        return -1;
    }

    if (lhs->day != rhs->day) {
        return (lhs->day < rhs->day) ? -1 : 1;
    }

    if (lhs->hour != rhs->hour) {
        return (lhs->hour < rhs->hour) ? -1 : 1;
    }

    if (lhs->min != rhs->min) {
        return (lhs->min < rhs->min) ? -1 : 1;
    }

    if (lhs->sec != rhs->sec) {
        return (lhs->sec < rhs->sec) ? -1 : 1;
    }

    if (lhs->nsec != rhs->nsec) {
        return (lhs->nsec < rhs->nsec) ? -1 : 1;
    }

    return 0;
}

int la_time_to_ns(uint64_t *out, const la_time_t *time) {
    if (!out || !time) {
        errno = EFAULT;
        return -1;
    }

    if (time->nsec < 0 || time->nsec >= LA_NS_PER_SEC ||
        time->sec  < 0 || time->sec  >= LA_SEC_PER_MIN ||
        time->min  < 0 || time->min  >= LA_MIN_PER_HOUR ||
        time->hour < 0 || time->hour >= LA_HOUR_PER_DAY) {
        errno = EINVAL;
        return -1;
    }

    int64_t total = 0;
    int64_t tmp;

    /* d -> s */
    if (__builtin_mul_overflow(time->day, LA_HOUR_PER_DAY, &tmp) ||
        __builtin_mul_overflow(tmp, LA_MIN_PER_HOUR, &tmp) ||
        __builtin_mul_overflow(tmp, LA_SEC_PER_MIN, &tmp) ||
        __builtin_add_overflow(total, tmp, &total)) {
        errno = ERANGE;
        return -1;
    }

    /* h -> s */
    if (__builtin_mul_overflow(time->hour, LA_MIN_PER_HOUR, &tmp) ||
        __builtin_mul_overflow(tmp, LA_SEC_PER_MIN, &tmp) ||
        __builtin_add_overflow(total, tmp, &total)) {
        errno = ERANGE;
        return -1;
    }

    /* m → s */
    if (__builtin_mul_overflow(time->min, LA_SEC_PER_MIN, &tmp) ||
        __builtin_add_overflow(total, tmp, &total)) {
        errno = ERANGE;
        return -1;
    }

    /* s */
    if (__builtin_add_overflow(total, time->sec, &total)) {
        errno = ERANGE;
        return -1;
    }

    /* s → nsec */
    if (__builtin_mul_overflow(total, LA_NS_PER_SEC, &total) ||
        __builtin_add_overflow(total, time->nsec, &total)) {
        errno = ERANGE;
        return -1;
        }

    if (total < 0) {
        errno = ERANGE;
        return -1;
    }

    *out = (uint64_t)total;
    return 0;
}

void la_time_from_ns(la_time_t *time, uint64_t ns) {}

uint64_t la_time_to_ptp(const la_time_t *time) {}

uint32_t la_time_to_rtp(const la_time_t *time, uint32_t rate) {}

double la_time_to_double(const la_time_t *time) {}

void la_time_from_double(la_time_t *time, double secs) {}

void la_time_from_ptp(la_time_t *time, uint64_t ptpns) {}
