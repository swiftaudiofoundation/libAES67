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
 * @file include/libAES67/__posix_tz.h
 * @brief
 */


#ifndef LIBAES67___POSIX_TZ_H
#define LIBAES67___POSIX_TZ_H

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "platform.h"
#include "time.h"

__LA_BEGIN_C_DECLS

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-identifier"

#define LA_TZ_MAX_NAME_LEN      32
#define LA_TZ_MAX_RULE_LEN      64
#define LA_SECONDS_PER_HOUR     3600
#define LA_WEEKDAY_COUNT        7
#define LA_LAST_WEEK            (-1)

LA_INLINE int __la_parse_signed_int(const char **ptr) {
    const char *str = *ptr;
    int sign = 1;
    if (*str == '-') { sign = -1; str++; }
    else if (*str == '+') { sign = 1; str++; }

    if (!isdigit((unsigned char)*str)) { return 0; }

    int value = 0;
    while (isdigit((unsigned char)*str)) {
        value = (value * 10) + (*str - '0');
        str++;
    }

    *ptr = str;
    return value * sign;
}

LA_INLINE int __la_parse_time(const char **ptr, int *hour, int *minutes, int *seconds) {
    *hour = 0;
    *minutes = 0;
    *seconds = 0;

    if (!ptr || !*ptr) { return -1; }

    *hour = __la_parse_signed_int(ptr);

    if (**ptr == ':') {
        (*ptr)++;
        *minutes = __la_parse_signed_int(ptr);

        if (**ptr == ':') {
            (*ptr)++;
            *seconds = __la_parse_signed_int(ptr);
        }
    }

    return 0;
}

LA_INLINE int __la_parse_dst_rule(const char *rule_str, int *month, int *week,
                                  int *weekday, int *hour, int *seconds) {
    if (rule_str[0] != 'M') { return 0; }

    const char *ptr = rule_str + 1;

    *month = __la_parse_signed_int(&ptr);
    if (*ptr != '.') { return -1; }
    ptr++;

    *week = __la_parse_signed_int(&ptr);
    if (*ptr != '.') { return -1; }
    ptr++;

    *weekday = __la_parse_signed_int(&ptr);

    int _hour = 2;
    int _minutes = 0;
    int _seconds = 0;
    if (*ptr == '/') {
        ptr++;
        __la_parse_time(&ptr, &_hour, &_minutes, &_seconds);
    }

    *hour = _hour;
    *seconds = (_hour * 3600) + (_minutes * 60) + _seconds;

    return 0;
}

LA_INLINE int __la_parse_posix_tz(la_timezone_t *tz, const char *tzstring) {
    if (!tz || !tzstring) {
        if (tz) {
            tz->error_code = 1;
            tz->error_message = strdup("NULL timezone pointer or TZ string");
        }
        return -1;
    }

    const char *ptr = tzstring;

    size_t name_len = 0;
    while (isalpha((unsigned char)ptr[name_len]) && name_len < LA_TZ_MAX_NAME_LEN - 1) {
        name_len++;
    }

    if (name_len == 0) {
        tz->error_code = 2;
        tz->error_message = strdup("Missing or invalid standard timezone name");
    }

    char std_name[LA_TZ_MAX_NAME_LEN] = {0};
    strncpy(std_name, ptr, name_len);
    std_name[name_len] = '\0';
    tz->name = strdup(std_name);
    if (!tz->name) {
        tz->error_code = 3;
        tz->error_message = strdup("Could not allocate memory for timezone name");
        return -1;
    }

    ptr += name_len;

    const int hours = __la_parse_signed_int(&ptr);
    int minutes = 0;
    int seconds = 0;
    if (*ptr == ':') { ptr++; minutes = __la_parse_signed_int(&ptr); }
    if (*ptr == ':') { ptr++; seconds = __la_parse_signed_int(&ptr); }

    tz->offset_sec = (hours * LA_SECONDS_PER_HOUR) + minutes * 60 + seconds;

    if (isalpha((unsigned char)*ptr)) {
        const char *dst_name_start = ptr;
        while (isalpha((unsigned char)*ptr)) { ptr++; }
        char dst_name[LA_TZ_MAX_NAME_LEN] = {0};
        strncpy(dst_name, dst_name_start, ptr - dst_name_start);
        dst_name[ptr - dst_name_start] = '\0';

        tz->dst_offset_sec = tz->offset_sec + LA_SECONDS_PER_HOUR;
        tz->is_dst = 1;

        if (*ptr == '+' || *ptr == '-') {
            const int dst_hours = __la_parse_signed_int(&ptr);
            int dst_minutes = 0;
            int dst_seconds = 0;
            if (*ptr == ':') { ptr++; dst_minutes = __la_parse_signed_int(&ptr); }
            if (*ptr == ':') { ptr++; dst_seconds = __la_parse_signed_int(&ptr); }
            tz->dst_offset_sec = dst_hours * LA_SECONDS_PER_HOUR + dst_minutes * 60 + dst_seconds;
        }

        if (*ptr == ',') {
            ptr++;
            const char *comma = strchr(ptr, ',');
            if (!comma) {
                tz->error_code = 4;
                tz->error_message = strdup("Invalid DST rules: missing comma seperator");
                free((void*)tz->name);
                return -1;
            }

            char start_rule[LA_TZ_MAX_RULE_LEN] = {0};
            char end_rule[LA_TZ_MAX_RULE_LEN]   = {0};

            strncpy(start_rule, ptr, (size_t)(comma - ptr));
            start_rule[comma - ptr] = '\0';

            strncpy(end_rule, comma + 1, LA_TZ_MAX_RULE_LEN -1);
            end_rule[LA_TZ_MAX_RULE_LEN - 1] = '\0';

            if (__la_parse_dst_rule(start_rule,
                    &tz->dst_start_month,
                    &tz->dst_start_week,
                    &tz->dst_start_weekday,
                    &tz->dst_start_hour,
                    &tz->dst_start_seconds) != 0 ||
                __la_parse_dst_rule(end_rule,
                    &tz->dst_end_month,
                    &tz->dst_end_week,
                    &tz->dst_end_weekday,
                    &tz->dst_end_hour,
                    &tz->dst_end_seconds) != 0) {
                tz->error_code = 5;
                tz->error_message = strdup("Failed to parse DST start or end rule");
                free((void*)tz->name);
                tz->name = NULL;
                return -1;
            }
        } else {
            /* No rules provided */
            tz->dst_start_month = tz->dst_end_month = 0;
        }
    } else {
        tz->is_dst = 0;
        tz->dst_offset_sec = 0;
        tz->dst_start_month = 0;
        tz->dst_start_week = 0;
        tz->dst_start_weekday = 0;
        tz->dst_start_hour = 0;
        tz->dst_start_seconds = 0;

        tz->dst_end_month = 0;
        tz->dst_end_week = 0;
        tz->dst_end_weekday = 0;
        tz->dst_end_hour = 0;
        tz->dst_end_seconds = 0;
    }

    tz->error_code = 0;
    if (tz->error_message) {
        free(tz->error_message);
        tz->error_message = NULL;
    }

    return 0;
}

#pragma clang diagnostic pop

__LA_END_C_DECLS

#endif /* LIBAES67___POSIX_TZ_H */
