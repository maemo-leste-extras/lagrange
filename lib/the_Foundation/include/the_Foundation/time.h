#pragma once

/** @file the_Foundation/time.h  Time and date manipulation.

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

#include "stream.h"
#include "string.h"
#include <time.h>

iBeginPublic

iDeclareType(Time)
iDeclareType(Date)
iDeclareTypeSerialization(Date)

struct Impl_Time {
    struct timespec ts;
};

enum iDateWeekday {
    sunday_DateWeekday,
    monday_DateWeekday,
    tuesday_DateWeekday,
    wednesday_DateWeekday,
    thursday_DateWeekday,
    friday_DateWeekday,
    saturday_DateWeekday,
};

struct Impl_Date {
    int year;
    short month;
    short day;
    short dayOfYear;
    short hour;
    short minute;
    short second;
    long nsecs;
    long gmtOffsetSeconds;
    iBool isDST;
    enum iDateWeekday dayOfWeek;
};

void    init_Time           (iTime *, const iDate *);
void    initCurrent_Time    (iTime *);
void    initSeconds_Time    (iTime *, double seconds);
void    initTimeout_Time    (iTime *, double seconds);

iTime       now_Time        (void);
double      seconds_Time    (const iTime *);
iString *   format_Time     (const iTime *, const char *format);

#define isValid_Time(d)         ((d)->ts.tv_sec > 0)
#define integralSeconds_Time(d) ((d)->ts.tv_sec)
#define nanoSeconds_Time(d)     ((d)->ts.tv_nsec)

void    add_Time            (iTime *, const iTime *time);
void    sub_Time            (iTime *, const iTime *time);
int     cmp_Time            (const iTime *, const iTime *);
void    max_Time            (iTime *, const iTime *time);

void    init_Date           (iDate *, const iTime *);
void    initCurrent_Date    (iDate *);
void    initSinceEpoch_Date (iDate *, time_t seconds);
void    initStdTime_Date    (iDate *, const struct tm *);

time_t  sinceEpoch_Date     (const iDate *);

iLocalDef double elapsedSeconds_Time(const iTime *d) {
    iTime elapsed = now_Time();
    sub_Time(&elapsed, d);
    return seconds_Time(&elapsed);
}
iLocalDef double secondsSince_Time(const iTime *d, const iTime *olderTime) {
    iTime dt = *d;
    sub_Time(&dt, olderTime);
    return seconds_Time(&dt);
}

iString *   format_Date     (const iDate *, const char *format);

iEndPublic
