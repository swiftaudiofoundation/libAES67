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
 * @file include/libAES67/time.h
 * @brief Clock and time abstraction for PTP.
 */

/*
 * Definition inspired by the time interface proposed by
 * Markus Kuhn, University of Cambridge Computer Laboratory:
 *
 *   <https://www.cl.cam.ac.uk/~mgk25/time/c/>
 */

#ifndef LIBAES67_TIME_H
#define LIBAES67_TIME_H

#define LIBAES67_INTERNAL_INCLUDE 1

#include <stdint.h>

#include "platform.h"
#include "__tai.h"

#define LA_NS_PER_SEC  1000000000LL

/**
 * @def LA_CLOCK_1588
 * @brief Alias for PTP clock.
 */
#define LA_CLOCK_1588      LA_CLOCK_PTP

/**
 * @def LA_CLOCK_1588_2008
 * @brief Alias for IEEE 1588-2008 (PTPv2) clock.
 */
#define LA_CLOCK_1588_2008 LA_CLOCK_PTPv2

__LA_BEGIN_C_DECLS

/**
 * @struct la_time_t
 * @brief Represents a point in time relative to a reference epoch.
 *
 * Time is stored as seconds and nanoseconds relative to an epoch (e.g., UNIX epoch or PTP epoch).
 * This structure supports high-precision time measurement suitable for audio streaming,
 * network synchronization, and real-time applications.
 */
typedef struct {
    /**
     * Identifies a one-second-long time interval that starts sec
     * seconds after the epoch.
     */
    int_fast64_t sec;

    /**
     * Contains the number of completed nanoseconds from the start of
     * the second interval identified by sec to the described point
     * in time. Values will always be in range 0 to 999_999_999.
     */
    int_fast32_t nsec;
} la_time_t;

// TODO: Add CLOCK_BOOTTIME => CLOCK_UPTIME, CLOCK_MONOTONIC_COARSE => MONOTONIC_RAW
/**
 * @enum la_clock_t
 * @brief Enumeration of supported clock sources.
 */
typedef enum {
    /**
     * Coordinated Universal Time (UTC)
     *
     * Counts non-leap seconds since the epoch. Leap seconds are
     * represented by extending the nanosecond field beyond 1e9
     * during the leap second.
     */
    LA_CLOCK_UTC,

    /**
     * International Atomic Time (TAI)
     *
     * Counts SI seconds continuously without leap-seconds
     * discontinuities. TAI clocks always advance monotonically
     * and include leap seconds.
     */
	LA_CLOCK_TAI,

	/**
     * Monotonic system clock
     *
     * Starts at an unspecified time before system boot and
     * increases continuously. Monotonic clocks are not affected
     * by system clock adjustments or leap seconds.
     */
    LA_CLOCK_MONOTONIC,

    /**
     * CPU time consumed by the current process
     *
     * Starts at an unspecified time during the generation of
     * the current process.
     */
    LA_CLOCK_PROCESS,

    /**
     * CPU time consumed by the current thread
     *
     * Starts at an unspecified time during the generation of
     * the current thread.
     */
    LA_CLOCK_THREAD,

    /**
     * IEEE 1588-2002 Precision Time Protocol clock.
     */
    LA_CLOCK_PTP,

    /**
     * IEEE 1588-2008 Precision Time Protocol clock (PTPv2)
     */
    LA_CLOCK_PTPv2
} la_clock_t;

/**
 * @struct la_ptp_time_t
 * @brief Represents PTP-specific time with fractional nanoseconds.
 *
 * Provides extended precision for IEEE 1588-2008 synchronization.
 */
typedef struct {
    int64_t sec;      /**< Seconds part of PTP time. */
    int64_t nsec;     /**< Nanoseconds part of PTP time. */
    int32_t frac;     /**< Fractional nanoseconds for sub-nanosecond accuracy. */
} la_ptp_time_t;

/**
 * @brief Retrieve the current time for the specified clock.
 *
 * @param xtp Pointer to la_time_t structure to store the result.
 * @param clock_type Clock source to query (see la_clock_t).
 * @return On success, if the requested clock type was available,
 *         the function shall return a value `r` with `(r & clock_type) == clock_type`.
 *         If `xtp != NULL` then the value of the requested clock shall be stored in `*xtp`.
 *         If the requested clock type was not available or the requested clock type was not
 *         known, the function shall return a value `r` with `(r & clock_type) == 0` and
 *         `*xtp` will be undefined.
 *
 * Standards: IEEE 1588-2008, POSIX clock_gettime().
 */
int la_time_get(la_time_t *xtp, la_clock_t clock_type);

/**
 * @brief Retrieve the resolution of a specified clock.
 *
 * @param res Pointer to la_time_t to store clock resolution.
 * @param clock_type Clock source (see la_clock_t).
 * @return 0 on success, non-zero on error.
 *
 * The resolution indicates the smallest measurable interval for the clock.
 */
int la_time_getres(la_time_t *res, la_clock_t clock_type);

/**
 * @brief Sleep for a specified duration.
 *
 * @param duration Duration to sleep (seconds + nanoseconds).
 * @return 0 on success, non-zero on error.
 *
 * The function shall wait for at least `sec + nsec / 10^9` seconds on the `TIME_MONOTONIC`
 * clock. The function shall return immediately if this value is not positive.
 *
 * Standards: POSIX nanosleep().
 *
 * See: <https://man7.org/linux/man-pages/man2/nanosleep.2.html>
 */
int la_time_sleep(const la_time_t *duration);

/**
 * @brief Sleep until a specified absolute time.
 *
 * @param target Absolute target time.
 * @param clock_type Clock source to use (e.g., monotonic, PTP).
 * @return 0 on success, non-zero on error.
 */
int la_time_sleep_until(const la_time_t *target, la_clock_t clock_type);

/**
 * @brief Convert time between different clocks.
 *
 * @param dst Destination time.
 * @param dst_clock_type Destination clock type.
 * @param src Source time.
 * @param src_clock_type Source clock type.
 * @return 0 on success, -1 and `*dst` undefined on error.
 *
 * Converts struct @p la_time_t values between different clock types and gives this way the
 * application access to the information that the implementation has available about the
 * relationship between the various clock types. The value `*src` as it would have been
 * returned by clock type `src_clock_type` is converted into the value that would at the
 * same time have been returned by clock type `dst_clock_type` and stored in `*dst`.
 */
int la_time_conv(la_time_t *dst, int dst_clock_type, const la_time_t *src, int src_clock_type);

/**
 * @brief Normalize a @p la_time_t so that nsec is within 0..999,999,999.
 *
 * @param xtp Time structure to normalize.
 * @return 0 on success.
 */
int la_time_normalize(la_time_t *xtp);

/**
 * @brief Add two @p la_time_t values.
 *
 * @param dst Result of lhs + rhs.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return 0 on success.
 */
int la_time_add(la_time_t *dst, const la_time_t *lhs, const la_time_t *rhs);

/**
 * @brief Subtract two @p la_time_t values.
 *
 * @param dst Result of lhs - rhs.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return 0 on success.
 */
int la_time_sub(la_time_t *dst, const la_time_t *lhs, const la_time_t *rhs);

/**
 * @brief Compare two @p la_time_t values.
 *
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Negative if lhs < rhs, zero if equal, positive if lhs > rhs.
 */
int la_time_cmp(const la_time_t *lhs, const la_time_t *rhs);

/**
 * @brief Convert @p la_time_t to total nanoseconds.
 *
 * @param time Input time.
 * @return Total nanoseconds as uint64_t.
 */
uint64_t la_time_to_ns(const la_time_t *time);

/**
 * @brief Initialize @p la_time_t from nanoseconds.
 *
 * @param time Output time.
 * @param ns Total nanoseconds.
 */
void la_time_from_ns(la_time_t *time, uint64_t ns);

/**
 * @brief Convert @p la_time_t to PTP representation (nanoseconds since PTP epoch).
 *
 * @param time Input time.
 * @return PTP time in nanoseconds.
 */
uint64_t la_time_to_ptp(const la_time_t *time);

/**
 * @brief Convert @p la_time_t to RTP timestamp.
 *
 * @param time Input time.
 * @param rate Sampling rate or clock rate (Hz).
 * @return RTP timestamp as 32-bit value.
 */
uint32_t la_time_to_rtp(const la_time_t *time, uint32_t rate);

/**
 * @brief Convert @p la_time_t to double-precision seconds.
 *
 * @param time Input time.
 * @return Seconds as double.
 */
double la_time_to_double(const la_time_t *time);

/**
 * @brief Initialize @p la_time_t from double-precision seconds.
 *
 * @param time Output time.
 * @param secs Seconds as double.
 */
void la_time_from_double(la_time_t *time, double secs);

/**
 * @brief Convert PTP nanoseconds to @p la_time_t.
 *
 * @param time Output time.
 * @param ptpns PTP timestamp in nanoseconds.
 */
void la_time_from_ptp(la_time_t *time, uint64_t ptpns);

__LA_END_C_DECLS

#undef LIBAES67_INTERNAL_INCLUDE

#endif /* LIBAES67_TIME_H */
