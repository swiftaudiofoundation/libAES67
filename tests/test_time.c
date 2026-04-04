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
 * test/test_time.c
 * Description: Tests for clock and time abstraction.
 */

#define LIBAES67_INTERNAL_INCLUDE 1

#include <libAES67/platform.h>
#include <libAES67/time.h>
#include <libAES67/__tai.h>

#include <time.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <inttypes.h>

// TODO: Refactor argument order to be ascending: *t, nsec, .., day
static void la_test_assert_time_eq(const la_time_t *t,
                                   const int_fast32_t day,
                                   const int_fast32_t hour,
                                   const int_fast32_t min,
                                   const int_fast64_t sec,
                                   const int_fast32_t nsec) {
    if (t->day != day || t->hour != hour || t->min != min ||
        t->sec != sec || t->nsec != nsec) {
        fprintf(stderr,
            "Time mismatch:\n"
            " got: d=%" PRIdFAST32 " h=%" PRIdFAST32 " m=%" PRIdFAST32
            " s=%" PRIdFAST64 " ns=%" PRIdFAST32 "\n"
            " exp: d=%" PRIdFAST32 " h=%" PRIdFAST32 " m=%" PRIdFAST32
            " s=%" PRIdFAST64 " ns=%" PRIdFAST32 "\n",
            t->day, t->hour, t->min, t->sec, t->nsec,
            day, hour, min, sec, nsec);
        assert(0);
    }
}

static void test_la_time_init(void) {
    la_time_t t = {0};
    la_test_assert_time_eq(&t, 0, 0, 0, 0, 0);
}

/*
 * Provide test cases for functions:
 *    - static void test_la_utc_to_tai(void) {}
 *    - static void test_la_tai_to_utc(void) {}
 *    - static void test_la_utc_ns_to_tai_ns(void) {}
 *    - static void test_la_tai_ns_to_utc_ns(void) {}
 */

//===========================================================================
static void test_la_time_get_utc_basic(void) {
    printf("== la_time_get UTC basic\n");

    la_time_t xtp;
    assert(la_time_get(&xtp, LA_CLOCK_UTC) == 0);

    assert(xtp.nsec >= 0 && xtp.nsec < LA_NS_PER_SEC);
    assert(xtp.sec  >= 0 && xtp.sec  < LA_SEC_PER_MIN);
    assert(xtp.min  >= 0 && xtp.min  < LA_MIN_PER_HOUR);
    assert(xtp.hour >= 0 && xtp.hour < LA_HOUR_PER_DAY);
    assert(xtp.day  >= 0);
}

static void test_la_time_get_monotonic_progres(void) {
    printf("== la_time_get monotonic progression\n");

    la_time_t a = {0}, b = {0};

    assert(la_time_get(&a, LA_CLOCK_MONOTONIC) == 0);

    /* Wait 1 ms for time to advance */
    const struct timespec req = { .tv_sec = 0, .tv_nsec = 1 * 1000 * 1000 };
    nanosleep(&req, NULL);

    assert(la_time_get(&b, LA_CLOCK_MONOTONIC) == 0);

    la_time_normalize(&a);
    la_time_normalize(&b);

    /* Expect monotonic progress */
    assert(la_time_cmp(&b, &a) > 0);
}

//===========================================================================
static void test_la_time_getres_utc(void) {
    printf("== Resolution UTC\n");

    la_time_t res;
    const int ret = la_time_getres(&res, LA_CLOCK_UTC);

    assert(ret == 0);

    assert(res.day  >= 0);
    assert(res.hour >= 0);
    assert(res.min  >= 0);
    assert(res.sec  >= 0);
    assert(res.nsec >= 0);

    assert(res.nsec < LA_NS_PER_SEC);
    assert(res.sec  < 60);
    assert(res.min  < 60);
    assert(res.hour < 24);

    assert(!(res.day == 0 &&
             res.hour == 0 &&
             res.min == 0 &&
             res.sec == 0 &&
             res.nsec == 0));
}

static void test_la_time_getres_tai(void) {
    printf("== Resolution TAI\n");

    la_time_t utc, tai;

    assert(la_time_getres(&utc, LA_CLOCK_UTC) == 0);
    assert(la_time_getres(&tai, LA_CLOCK_TAI) == 0);

    /*
     * Resolution should be identical between UTC and TAI,
     * since they differ only by a constant offset.
     */
    assert(utc.nsec == tai.nsec);
    assert(utc.sec  == tai.sec);
}

static void test_la_time_getres_null(void) {
    printf("== Resolution NULL\n");

    assert(la_time_getres(NULL, LA_CLOCK_UTC) == -1);
}

static void test_la_time_getres_invalid_clock(void) {
    printf("== Resolution Invalid clock & sys comparison\n");

    la_time_t res;
    const int ret = la_time_getres(&res, (la_clock_t)999);

    assert(ret == -1);
}

static void test_la_time_getres_utc_vs_system(void) {
    printf("== Resolution UTC vs system\n");

    la_time_t res;
    struct timespec ts;

    assert(la_time_getres(&res, LA_CLOCK_UTC) == 0);
    assert(clock_getres(CLOCK_REALTIME, &ts) == 0);

    assert(res.sec == ts.tv_sec);
    assert(res.nsec == ts.tv_nsec);
}

//===========================================================================
static void test_la_time_conv_utc_to_tai_1970(void) {
    printf("== Conversion UTC-TAI 1970\n");

    la_time_t dst;
    const la_time_t src = { .nsec = 0, .sec = 30, .min = 1, .hour = 1, .day = 0 };
    const int ret = la_time_conv(&dst, LA_CLOCK_TAI, &src, LA_CLOCK_UTC);

    assert(ret == 0);
    /*
     * la_time_t represents a timestamp relative to the Unix epoch
     * (1970-01-01 00:00:00 UTC). At that point, the TAI-UTC offset
     * was the initial +10 seconds as defined by the IERS Bulletin C.
     *
     * Therefore:
     *      1970-01-01 01:01:30 UTC + 10 sec TAI offset => 1970-01-01 01:01:40 TAI
     *
     * Reference:
     *      - TAI Source <libAES67/__tai.h>
     *      - IERS Bulletin C / NTP leap-seconds.list
     */
    la_test_assert_time_eq(&dst, 0, 1, 1, 40, 0);
}

static void test_la_time_conv_tai_to_utc_1970(void) {
    printf("== Conversion TAI-UTC 1970\n");

    la_time_t dst;
    const la_time_t src = { .nsec = 0, .sec = 40, .min = 1, .hour = 1, .day = 0 };
    const int ret = la_time_conv(&dst, LA_CLOCK_UTC, &src, LA_CLOCK_TAI);

    assert(ret == 0);
    /*
     * Inverse of the above conversion. At epoch, TAI-UTC = +10s,
     * so the offset is subtracted when converting back to UTC.
     *
     *      1970-01-01 01:01:40 TAI - 10 sec TAI offset => 1970-01-01 01:01:30 UTC
     */
    la_test_assert_time_eq(&dst, 0, 1, 1, 30, 0);
}

static void test_la_time_conv_utc_to_tai_2017(void) {
    printf("== Conversion UTC-TAI-2017\n");

    la_time_t dst;
    const la_time_t src = { .nsec = 0, .sec = 1483228800, .min = 0, .hour = 0, .day = 0 };
    const int ret = la_time_conv(&dst, LA_CLOCK_TAI, &src, LA_CLOCK_UTC);

    assert(ret == 0);
    /*
     * 2017-01-01 00:00:00 UTC + 37 sec => 2017-01-01 00:00:37 TAI
     */
    la_test_assert_time_eq(&dst, 17167, 0, 0, 37, 0);
}

static void test_la_time_conv_tai_to_utc_2017(void) {
    printf("== Conversion TAI-UTC-2017\n");

    la_time_t dst;
    const la_time_t src = { .nsec = 0, .sec = 1483228800, .min = 0, .hour = 0, .day = 0 };
    const int ret = la_time_conv(&dst, LA_CLOCK_UTC, &src, LA_CLOCK_TAI);

    assert(ret == 0);
    /*
     * 2017-01-01 00:00:00 TAI - 37 sec => 2016-12-31 23:59:24 UTC
     */
    la_test_assert_time_eq(&dst, 17166, 23, 59, 24, 0);
}

static void test_la_time_conv_utc_to_tai_leap_transition_2017(void) {
    printf("== Conversion UTC-TAI-LEAP-transition\n");

    const la_time_t before = { .sec = 1483228799 }; /* 2016-12-31 23:59:59 */
    const la_time_t after  = { .sec = 1483228800 }; /* 2017-01-01 00:00:00 */

    la_time_t dst_before, dst_after;

    la_time_conv(&dst_before, LA_CLOCK_TAI, &before, LA_CLOCK_UTC);
    la_time_conv(&dst_after,  LA_CLOCK_TAI, &after,  LA_CLOCK_UTC);

    /* Diffrence should be 2 seconds (1 real second and 1 leap second) */
    int64_t ns_before, ns_after;
    la_time_to_ns((uint64_t *)&ns_before, &dst_before);
    la_time_to_ns((uint64_t *)&ns_after,  &dst_after);

    assert(ns_after - ns_before == 2 * LA_NS_PER_SEC);
}

static void test_la_time_conv_same_clk_types(void) {
    printf("== Same clock types\n");

    la_time_t dst;
    const la_time_t src = { .sec = 1483228800 }; /* 2017-01-01 00:00:00 UTC */
    const int ret = la_time_conv(&dst, LA_CLOCK_UTC, &src, LA_CLOCK_UTC);

    assert(ret == 0);
    assert(memcmp(&dst, &src, sizeof(la_time_t)) == 0);
}

static void test_la_time_conv_relative_to_absolute_error(void) {
    printf("== ERROR Relative to absolute conversion\n");

    la_time_t dst;
    const la_time_t src = { .sec = 1483228800 }; /* 2017-01-01 00:00:00 */
    const int ret = la_time_conv(&dst, LA_CLOCK_MONOTONIC, &src, LA_CLOCK_UTC);

    assert(ret == -1); /* Fails due to la_clock_is_relative check */
}

//===========================================================================
static void test_la_time_normalize(void) {
    printf("== Basic Normalization\n");

    la_time_t xtp = { .nsec = 0, .sec = 30, .min = 30, .hour = 1, .day = 0 };
    const int ret = la_time_normalize(&xtp);

    assert(ret == 0);
    la_test_assert_time_eq(&xtp, 0, 1, 30, 30, 0);
}

static void test_la_time_normalize_input(void) {
    printf("== Normalize la_time_t value\n");

    la_time_t xtp = { .nsec = 0, .sec = 70, .min = 70, .hour = 25, .day = 0 };
    const int ret = la_time_normalize(&xtp);

    assert(ret == 0);
    la_test_assert_time_eq(&xtp, 1, 2, 11, 10, 0);
}

static void test_la_time_normalize_out_of_range(void) {
    printf("== ERROR Out of Range\n");

    la_time_t xtp = { .nsec = 0, .sec = -70, .min = -70, .hour = -25, .day = 0 };
    const int ret = la_time_normalize(&xtp);

    assert(ret == -1);
    assert(errno == ERANGE);
}

static void test_la_time_normalize_null_ptr_error(void) {
    printf("== ERROR NULL ptr\n");

    const int ret = la_time_normalize(NULL);

    assert(ret == -1);
    assert(errno == EFAULT);
}

//===========================================================================
static void test_la_time_add(void) {
    printf("== Basic addition\n");

    la_time_t dst;
    const la_time_t val = { .nsec = 0, .sec = 0, .min = 30, .hour = 1, .day = 0 };
    const int ret = la_time_add(&dst, &val, &val);

    assert(ret == 0);
    la_test_assert_time_eq(&dst, 0, 3, 0, 0, 0);
}

static void test_la_time_add_null_ptr_error(void) {
    printf("== ERROR NULL ptr\n");

    la_time_t dst;
    const int ret = la_time_add(&dst, NULL, NULL);

    assert(ret == -1);
    assert(errno == EFAULT);
}

//===========================================================================
static void test_la_time_sub(void) {
    printf("== Basic subtraction\n");

    la_time_t dst;
    const la_time_t lhs = { .nsec = 0, .sec = 20, .min = 4, .hour = 2, .day = 0 };
    const la_time_t rhs = { .nsec = 0, .sec = 20, .min = 2, .hour = 1, .day = 0 };
    const int ret = la_time_sub(&dst, &lhs, &rhs);

    assert(ret == 0);
    la_test_assert_time_eq(&dst, 0, 1, 2, 0, 0);
}

static void test_la_time_sub_negative(void) {
    printf("== Negative la_time_t\n");

    la_time_t dst;
    const la_time_t lhs = { .nsec = 0, .sec = 0, .min = 0, .hour = 0, .day = 0 };
    const la_time_t rhs = { .nsec = 1, .sec = 1, .min = 1, .hour = 1, .day = 1 };
    const int ret = la_time_sub(&dst, &lhs, &rhs);

    assert(ret == -1); /* underflow in la_time_t due to negative values */
    assert(errno == ERANGE);
}

static void test_la_time_sub_carry(void) {
    printf("== Carrying & Borrowing\n");

    la_time_t dst;
    const la_time_t lhs = { .nsec = 0, .sec = 30, .min = 6, .hour = 1, .day = 0 };
    const la_time_t rhs = { .nsec = 0, .sec = 40, .min = 0, .hour = 0, .day = 0 };
    const int ret = la_time_sub(&dst, &lhs, &rhs);

    assert(ret == 0);
    la_test_assert_time_eq(&dst, 0, 1, 5, 50, 0);
}

static void test_la_time_sub_carry_day(void) {
    printf("== Carrying & Borrowing (day)\n");

    la_time_t dst;
    const la_time_t lhs = { .nsec = 0, .sec = 0, .min = 0, .hour = 12, .day = 1 };
    const la_time_t rhs = { .nsec = 0, .sec = 0, .min = 0, .hour = 14, .day = 0  };
    const int ret = la_time_sub(&dst, &lhs, &rhs);

    assert(ret == 0);
    la_test_assert_time_eq(&dst, 0, 22, 0, 0, 0);
}

static void test_la_time_sub_ns(void) {
    printf("== Nanosecond-precision subtraction\n");

    la_time_t dst;
    const la_time_t lhs = { .nsec = LA_NS_PER_SEC, .sec = 0, .min = 0, .hour = 0, .day = 0 };
    const la_time_t rhs = { .nsec = LA_NS_PER_HALF_SEC, .sec = 0, .min = 0, .hour = 0, .day = 0 };
    const int ret = la_time_sub(&dst, &lhs, &rhs);

    assert(ret == 0);
    la_test_assert_time_eq(&dst, 0, 0, 0, 0, LA_NS_PER_HALF_SEC);
}

static void test_la_time_sub_null_ptr_error(void) {
    printf("== ERROR NULL ptr\n");

    la_time_t dst;
    const int ret = la_time_sub(&dst, NULL, NULL);

    assert(ret == -1);
    assert(errno == EFAULT);
}

//============================================================================
static void test_la_time_cmp_same_value(void) {
    printf("== Same Value Comparison\n");

    const la_time_t val = { .nsec = 0, .sec = 30, .min = 4, .hour = 1, .day = 0  };
    const int ret = la_time_cmp(&val, &val);

    assert(ret == 0);
}

static void test_la_time_cmp_different_values_lhs(void) {
    printf("== Different Value Comparison lhs\n");

    const la_time_t lhs = { .nsec = 1, .sec = 1, .min = 1, .hour = 1, .day = 1 };
    const la_time_t rhs = { .nsec = 0, .sec = 0, .min = 0, .hour = 0, .day = 0 };
    const int ret = la_time_cmp(&lhs, &rhs);

    assert(ret == 1); /* lhs larger than rhs */
}

static void test_la_time_cmp_different_values_rhs(void) {
    printf("== Different Value Comparison rhs\n");

    const la_time_t lhs = { .nsec = 0, .sec = 0, .min = 0, .hour = 0, .day = 0 };
    const la_time_t rhs = { .nsec = 1, .sec = 1, .min = 1, .hour = 1, .day = 1 };
    const int ret = la_time_cmp(&lhs, &rhs);

    assert(ret == -1); /* rhs larger than lhs */
}

static void test_la_time_cmp_null_ptr_error(void) {
    printf("== ERROR NULL ptr\n");

    const int ret = la_time_cmp(NULL, NULL);
    assert(ret == -1);
}

//=============================================================================
static void test_la_time_to_ns_basic_conversion(void) {
    printf("== Basic Conversion\n");

    uint64_t output;
    const la_time_t time = { .nsec = 0, .sec = 1, .min = 1, .hour = 1, .day = 1 };

    const int ret = la_time_to_ns(&output, &time);
    assert(ret == 0);

    const uint64_t expected_sec = LA_DAY(1) + LA_HOUR(1) + LA_MIN(1) + LA_SEC(1);
    const uint64_t expected_nsec = expected_sec * LA_NS_PER_SEC;

    assert(output == expected_nsec);
}

static void test_la_time_to_ns_null_ptr_error(void) {
    printf("== ERROR NULL ptr\n");

    uint64_t output;
    const int ret = la_time_to_ns(&output, NULL);

    assert(ret == -1);
    assert(errno == EFAULT);
}

//==============================================================================
static void test_la_time_from_ns_basic_conversion(void) {
    printf("== Basic Conversion\n");

    la_time_t t;
    const int ret = la_time_from_ns(&t, 1'500'000'000ULL);

    assert(ret == 0);
    la_test_assert_time_eq(&t, 0, 0, 0, 1, LA_NS_PER_HALF_SEC);
}

static void test_la_time_from_ns_large_values(void) {
    printf("== Large Values\n");

    la_time_t t;

    const uint64_t secs = LA_DAY(1) + LA_HOUR(2) + LA_MIN(3) + LA_SEC(4);
    const uint64_t ns = (secs * LA_NS_PER_SEC) + LA_NS_PER_HALF_SEC;
    const int ret = la_time_from_ns(&t, ns);

    assert(ret == 0);
    la_test_assert_time_eq(&t, 1, 2, 3, 4, LA_NS_PER_HALF_SEC);
}

static void test_la_time_from_ns_large_values_max(void) {
    printf("== Large Values MAX\n");

    la_time_t t;
    uint64_t ns = UINT64_MAX;
    const int ret = la_time_from_ns(&t, ns);

    assert(ret == 0);
    assert(t.nsec >= 0 && t.nsec < LA_NS_PER_SEC);
    assert(t.min >= 0 && t.min < 60);
    assert(t.hour >= 0 && t.hour < 24);
}

static void test_la_time_from_ns_large_values_one_day(void) {
    printf("== One day\n");

    la_time_t t;
    uint64_t ns = 24ULL * 3600 * LA_NS_PER_SEC;
    const int ret = la_time_from_ns(&t, ns);

    assert(ret == 0);
    la_test_assert_time_eq(&t, 1, 0, 0, 0, 0);
}

static void test_la_time_from_ns_off_by_one_one_ns_before(void) {
    printf("== Boundary 1ns before 1 sec\n");

    la_time_t t;
    uint64_t ns = LA_NS_PER_SEC - 1;
    const int ret = la_time_from_ns(&t, ns);

    assert(ret == 0);
    la_test_assert_time_eq(&t, 0, 0, 0, 0, LA_NS_PER_SEC - 1);
}

static void test_la_time_from_ns_off_by_one_one_exactly_one_sec(void) {
    printf("== Boundary 1 sec\n");

    la_time_t t;
    uint64_t ns = LA_NS_PER_SEC;
    const int ret = la_time_from_ns(&t, ns);

    assert(ret == 0);
    la_test_assert_time_eq(&t, 0, 0, 0, 1, 0);
}

static void test_la_time_from_ns_invariants(void) {
    printf("== Invariants\n");

    la_time_t t;
    uint64_t ns = 123456789012345ULL;
    const int ret = la_time_from_ns(&t, ns);

    assert(ret == 0);
    assert(t.nsec >= 0 && t.nsec < LA_NS_PER_SEC);
    assert(t.sec >= 0 && t.sec < LA_SEC_PER_MIN);
    assert(t.min >= 0 && t.min < 60);
    assert(t.hour >= 0 && t.hour < 24);
}

static void test_la_time_from_ns_null_ptr_error(void) {
    printf("== ERROR NULL ptr\n");

    const int ret = la_time_from_ns(NULL, 1000);
    assert(ret == -1);
    assert(errno == EFAULT);
}

//==============================================================================
int main(void) {
    test_la_time_init();

    printf("========= Running la_time_get =========\n");
    test_la_time_get_utc_basic();
    test_la_time_get_monotonic_progres();

    printf("======== Running la_time_getres =======\n");
    test_la_time_getres_utc();
    test_la_time_getres_tai();
    test_la_time_getres_null();
    test_la_time_getres_invalid_clock();
    test_la_time_getres_utc_vs_system();

    printf("========= Running la_time_conv ========\n");
    test_la_time_conv_utc_to_tai_1970();
    test_la_time_conv_tai_to_utc_1970();
    test_la_time_conv_utc_to_tai_2017();
    test_la_time_conv_tai_to_utc_2017();
    test_la_time_conv_utc_to_tai_leap_transition_2017();
    test_la_time_conv_same_clk_types();
    test_la_time_conv_relative_to_absolute_error();

    printf("====== Running la_time_normalize ======\n");
    test_la_time_normalize();
    test_la_time_normalize_input();
    test_la_time_normalize_out_of_range();
    test_la_time_normalize_null_ptr_error();

    printf("========= Running la_time_add =========\n");
    test_la_time_add();
    test_la_time_add_null_ptr_error();

    printf("========= Running la_time_sub =========\n");
    test_la_time_sub();
    test_la_time_sub_negative();
    test_la_time_sub_carry();
    test_la_time_sub_carry_day();
    test_la_time_sub_ns();
    test_la_time_sub_null_ptr_error();

    printf("========= Running la_time_cmp =========\n");
    test_la_time_cmp_same_value();
    test_la_time_cmp_different_values_lhs();
    test_la_time_cmp_different_values_rhs();
    test_la_time_cmp_null_ptr_error();

    printf("======== Running la_time_to_ns ========\n");
    test_la_time_to_ns_basic_conversion();
    test_la_time_to_ns_null_ptr_error();

    printf("======= Running la_time_from_ns =======\n");
    test_la_time_from_ns_basic_conversion();
    test_la_time_from_ns_large_values();
    test_la_time_from_ns_large_values_max();
    test_la_time_from_ns_large_values_one_day();
    test_la_time_from_ns_off_by_one_one_ns_before();
    test_la_time_from_ns_off_by_one_one_exactly_one_sec();
    test_la_time_from_ns_invariants();
    test_la_time_from_ns_null_ptr_error();

    printf("\nAll libAES67 time tests passed!\n");
    return 0;
}
