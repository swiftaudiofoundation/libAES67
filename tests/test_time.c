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
 * test/time.c
 * Description: Tests for clock and time abstraction.
 */

#define LIBAES67_INTERNAL_INCLUDE 1

#include <libAES67/platform.h>
#include <libAES67/time.h>
#include <libAES67/__tai.h>

#include <time.h>
#include <assert.h>
#include <errno.h>
#include <inttypes.h>

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
 *    - static void test_la_time_get(void) {}
 *    - static void test_la_time_getres(void) {}
 *    - static void test_la_time_conv(void) {}
 *    - static void test_la_time_normalize(void) {}
 *    - static void test_la_time_add(void) {}
 *    - static void test_la_time_sub(void) {}
 *    - static void test_la_time_cmp(void) {}
 *    - static void test_la_time_to_ns(void) {}
*/

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
    assert(t.day == 1);
    assert(t.hour == 0);
    assert(t.min == 0);
    assert(t.sec == 0);
    assert(t.nsec == 0);
}

static void test_la_time_from_ns_off_by_one_one_ns_before(void) {
    printf("== Boundary 1ns before 1 sec\n");

    la_time_t t;
    uint64_t ns = LA_NS_PER_SEC - 1;
    const int ret = la_time_from_ns(&t, ns);

    assert(ret == 0);
    assert(t.day == 0);
    assert(t.hour == 0);
    assert(t.min == 0);
    assert(t.sec == 0);
    assert(t.nsec == LA_NS_PER_SEC - 1);
}

static void test_la_time_from_ns_off_by_one_one_exactly_one_sec(void) {
    printf("== Boundary 1 sec\n");

    la_time_t t;
    uint64_t ns = LA_NS_PER_SEC;
    const int ret = la_time_from_ns(&t, ns);

    assert(ret == 0);
    assert(t.day == 0);
    assert(t.hour == 0);
    assert(t.min == 0);
    assert(t.sec == 1);
    assert(t.nsec == 0);
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

    printf("======= Running la_time_from_ns =======\n");
    test_la_time_from_ns_basic_conversion();
    test_la_time_from_ns_large_values();
    test_la_time_from_ns_large_values_max();
    test_la_time_from_ns_large_values_one_day();
    test_la_time_from_ns_off_by_one_one_ns_before();
    test_la_time_from_ns_off_by_one_one_exactly_one_sec();
    test_la_time_from_ns_invariants();
    test_la_time_from_ns_null_ptr_error();

    printf("All libAES67 time tests passed!\n");
    return 0;
}
