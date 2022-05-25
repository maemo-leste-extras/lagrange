/** @file time.c  Time and date manipulation.

@authors Copyright (c) 2017 Jaakko Keränen <jaakko.keranen@iki.fi>

@par License

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

<small>THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.</small>
*/

#include "the_Foundation/time.h"

#include <stdlib.h>
#include <math.h>

static time_t initStdTime_(const iDate *date, struct tm *tm) {
    tm->tm_year  = date->year - 1900;
    tm->tm_mon   = date->month - 1;
    tm->tm_mday  = date->day;
    tm->tm_hour  = date->hour;
    tm->tm_min   = date->minute;
    tm->tm_sec   = date->second;
    tm->tm_isdst = date->isDST ? 1 : 0;
#if !defined (iPlatformWindows) // we only know about daylight savings time 
    tm->tm_gmtoff = date->gmtOffsetSeconds;
#endif
    return mktime(tm);
}

void init_Time(iTime *d, const iDate *date) {
    struct tm tm;
    d->ts.tv_sec = initStdTime_(date, &tm);
    d->ts.tv_nsec = date->nsecs;
}

iTime now_Time(void) {
    iTime time;
    initCurrent_Time(&time);
    return time;
}

void initCurrent_Time(iTime *d) {
    clock_gettime(CLOCK_REALTIME, &d->ts);
}

void initSeconds_Time(iTime *d, double seconds) {
    double integral;
    double fractional = modf(seconds, &integral);
    d->ts.tv_sec  = (time_t) integral;
    d->ts.tv_nsec = (long)  (fractional * 1.0e9);
}

void initTimeout_Time(iTime *d, double seconds) {
    initSeconds_Time(d, seconds);
    add_Time(d, &(iTime){ now_Time().ts });
}

double seconds_Time(const iTime *d) {
    return (double) d->ts.tv_sec + (double) d->ts.tv_nsec / 1.0e9;
}

void add_Time(iTime *d, const iTime *time) {
    d->ts.tv_sec  += time->ts.tv_sec;
    d->ts.tv_nsec += time->ts.tv_nsec;
    while (d->ts.tv_nsec >= 1000000000L) {
        d->ts.tv_sec++;
        d->ts.tv_nsec -= 1000000000L;
    }
}

void sub_Time(iTime *d, const iTime *time) {
    d->ts.tv_sec  -= time->ts.tv_sec;
    d->ts.tv_nsec -= time->ts.tv_nsec;
    while (d->ts.tv_nsec < 0) {
        d->ts.tv_sec--;
        d->ts.tv_nsec += 1000000000L;
    }
}

int cmp_Time(const iTime *d, const iTime *other) {
    if (integralSeconds_Time(d) == integralSeconds_Time(other)) {
        return iCmp(nanoSeconds_Time(d), nanoSeconds_Time(other));
    }
    return iCmp(integralSeconds_Time(d), integralSeconds_Time(other));
}

void max_Time(iTime *d, const iTime *time) {
    if (integralSeconds_Time(time) > integralSeconds_Time(d)) {
        *d = *time;
    }
    else if (integralSeconds_Time(time) == integralSeconds_Time(d)) {
        d->ts.tv_nsec = iMax(d->ts.tv_nsec, time->ts.tv_nsec);                                       
    }
}

iString *format_Time(const iTime *d, const char *format) {
    iDate date;
    init_Date(&date, d);
    return format_Date(&date, format);
}

/*-------------------------------------------------------------------------------------*/

#if defined (iPlatformWindows)
iLocalDef void localtime_r(const time_t *sec, struct tm *t) {
    localtime_s(t, sec);
}
#endif

void initStdTime_Date(iDate *d, const struct tm *t) {
    d->year             = t->tm_year + 1900;
    d->month            = t->tm_mon + 1;
    d->day              = t->tm_mday;
    d->dayOfWeek        = t->tm_wday;
    d->dayOfYear        = t->tm_yday + 1;
    d->hour             = t->tm_hour;
    d->minute           = t->tm_min;
    d->second           = t->tm_sec;
    d->nsecs            = 0;
    d->isDST            = t->tm_isdst == 1;
#if !defined (iPlatformWindows)
    d->gmtOffsetSeconds = t->tm_gmtoff;
#else
    d->gmtOffsetSeconds = 0;
#endif
}

void initSinceEpoch_Date(iDate *d, time_t seconds) {
    struct tm t;
    localtime_r(&seconds, &t);
    initStdTime_Date(d, &t);
}

void init_Date(iDate *d, const iTime *time) {
    initSinceEpoch_Date(d, time->ts.tv_sec);
    d->nsecs = nanoSeconds_Time(time);
}

void initCurrent_Date(iDate *d) {
    const iTime now = now_Time();
    init_Date(d, &now);
}

static const uint16_t dstBit_Date_ = 0x8000;

void serialize_Date(const iDate *d, iStream *outs) {
    writeU16_Stream(outs, d->year);
    writeU8_Stream(outs, d->month);
    writeU8_Stream(outs, d->day);
    writeU16_Stream(outs, d->dayOfYear | (d->isDST ? dstBit_Date_ : 0));
    writeU8_Stream(outs, d->dayOfWeek);
    writeU8_Stream(outs, d->hour);
    writeU8_Stream(outs, d->minute);
    writeU8_Stream(outs, d->second);
    writeU32_Stream(outs, (uint32_t) d->nsecs);
    write16_Stream(outs, d->gmtOffsetSeconds);
}

void deserialize_Date(iDate *d, iStream *ins) {
    d->year = readU16_Stream(ins);
    d->month = readU8_Stream(ins);
    d->day = readU8_Stream(ins);
    uint16_t dayOfYear = readU16_Stream(ins);
    d->dayOfYear = dayOfYear & 0x1ff;
    d->isDST = (dayOfYear & dstBit_Date_) != 0;
    d->dayOfWeek = readU8_Stream(ins);
    d->hour = readU8_Stream(ins);
    d->minute = readU8_Stream(ins);
    d->second = readU8_Stream(ins);
    d->nsecs = readU32_Stream(ins);
    d->gmtOffsetSeconds = read16_Stream(ins);
}

time_t sinceEpoch_Date(const iDate *d) {
    struct tm time;
    initStdTime_(d, &time);
    return mktime(&time);
}

iString *format_Date(const iDate *d, const char *format) {
    struct tm time;
    initStdTime_(d, &time);
    iString *str = new_String();
    resize_Block(&str->chars, 63);
    size_t len;
    while ((len = strftime(data_Block(&str->chars), size_Block(&str->chars), format, &time)) == 0) {
        resize_Block(&str->chars, size_Block(&str->chars) * 2);
    }
    truncate_Block(&str->chars, len);
    return str;
}
