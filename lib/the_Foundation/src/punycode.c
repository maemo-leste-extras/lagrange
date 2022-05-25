/** @file punycode.c  Encoding/decoding IDNA Punycode according to RFC 3492.

@authors Copyright (c) 2020 Jaakko Keränen <jaakko.keranen@iki.fi>

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

/*

This code is derived from RFC 3492. (https://tools.ietf.org/html/rfc3492#appendix-C)

The original sample algorithm was written by Adam M. Costello
(http://www.nicemice.net/amc/). For more information, see
http://www.nicemice.net/idn/.

*/

#include "the_Foundation/string.h"

#include <stdint.h>
#include <unistr.h>

enum iPunyStatus {
    success_PunyStatus,
    badInput_PunyStatus,
    bigOutput_PunyStatus,
    overflow_PunyStatus,
};

enum iPunyParam {
    base_PunyParam        = 36,
    tmin_PunyParam        = 1,
    tmax_PunyParam        = 26,
    skew_PunyParam        = 38,
    damp_PunyParam        = 700,
    initialBias_PunyParam = 72,
    initialN_PunyParam    = 0x80,
    delimiter_PunyParam   = 0x2d,
};

iLocalDef iBool isBasic_    (uint32_t codepoint) { return codepoint < 0x80; }
iLocalDef iBool isDelim_    (uint32_t codepoint) { return codepoint == delimiter_PunyParam; }
iLocalDef iBool isFlagged_  (uint32_t bcp)       { return bcp - 65 < 26; }

static uint32_t decodeDigit_(uint32_t cp) {
    return cp - 48 < 10   ? cp - 22
           : cp - 65 < 26 ? cp - 65
           : cp - 97 < 26 ? cp - 97
                          : base_PunyParam;
}

static char encodeDigit_(uint32_t d, int flag) {
    return d + 22 + 75 * (d < 26) - ((flag != 0) << 5);
}

static char encodeBasic_(uint32_t bcp, int flag) {
    bcp -= (bcp - 97 < 26) << 5;
    return bcp + ((!flag && (bcp - 65 < 26)) << 5);
}

static uint32_t adapt_(uint32_t delta, uint32_t numPoints, iBool isFirstTime) {
    uint32_t k;
    delta = isFirstTime ? delta / damp_PunyParam : delta >> 1;
    delta += delta / numPoints;
    for (k = 0; delta > ((base_PunyParam - tmin_PunyParam) * tmax_PunyParam) / 2;
         k += base_PunyParam) {
        delta /= base_PunyParam - tmin_PunyParam;
    }
    return k + (base_PunyParam - tmin_PunyParam + 1) * delta / (delta + skew_PunyParam);
}

static enum iPunyStatus encode_(uint32_t inputLength, const uint32_t input[],
                                uint32_t *outputLength, char output[],
                                const unsigned char caseFlags[]) {
    uint32_t n      = initialN_PunyParam;
    uint32_t delta  = 0;
    uint32_t out    = 0;
    uint32_t outMax = *outputLength;
    uint32_t bias   = initialBias_PunyParam;

    uint32_t h, b, j, m, q, k, t;

    for (j = 0; j < inputLength; ++j) {
        if (isBasic_(input[j])) {
            if (outMax - out < 2) {
                return bigOutput_PunyStatus;
            }
            output[out++] = caseFlags ? encodeBasic_(input[j], caseFlags[j]) : input[j];
        }
    }

    h = b = out;

    if (b > 0) {
        output[out++] = delimiter_PunyParam;
    }

    while (h < inputLength) {
        for (m = UINT32_MAX, j = 0; j < inputLength; ++j) {
            if (input[j] >= n && input[j] < m) {
                m = input[j];
            }
        }
        if (m - n > (UINT32_MAX - delta) / (h + 1)) {
            return overflow_PunyStatus;
        }
        delta += (m - n) * (h + 1);
        n = m;
        for (j = 0; j < inputLength; ++j) {
            if (input[j] < n) {
                if (++delta == 0) {
                    return overflow_PunyStatus;
                }
            }
            if (input[j] == n) {
                for (q = delta, k = base_PunyParam;; k += base_PunyParam) {
                    if (out >= outMax) {
                        return bigOutput_PunyStatus;
                    }
                    t = k <= bias                    ? tmin_PunyParam
                        : k >= bias + tmax_PunyParam ? tmax_PunyParam
                                                     : k - bias;
                    if (q < t) {
                        break;
                    }
                    output[out++] = encodeDigit_(t + (q - t) % (base_PunyParam - t), 0);
                    q             = (q - t) / (base_PunyParam - t);
                }
                output[out++] = encodeDigit_(q, caseFlags && caseFlags[j]);
                bias          = adapt_(delta, h + 1, h == b);
                delta         = 0;
                ++h;
            }
        }
        ++delta, ++n;
    }

    *outputLength = out;
    return success_PunyStatus;
}

static enum iPunyStatus decode_(uint32_t inputLength, const char input[], uint32_t *outputLength,
                                uint32_t output[], unsigned char caseFlags[]) {
    uint32_t n      = initialN_PunyParam;
    uint32_t out    = 0;
    uint32_t i      = 0;
    uint32_t outMax = *outputLength;
    uint32_t bias   = initialBias_PunyParam;

    uint32_t b, j, in, oldi, w, k, digit, t;

    for (b = j = 0; j < inputLength; ++j) {
        if (isDelim_(input[j])) {
            b = j;
        }
    }
    if (b > outMax) {
        return bigOutput_PunyStatus;
    }

    for (j = 0; j < b; ++j) {
        if (caseFlags) {
            caseFlags[out] = isFlagged_(input[j]);
        }
        if (!isBasic_(input[j])) {
            return badInput_PunyStatus;
        }
        output[out++] = input[j];
    }

    for (in = b > 0 ? b + 1 : 0; in < inputLength; ++out) {
        for (oldi = i, w = 1, k = base_PunyParam;; k += base_PunyParam) {
            if (in >= inputLength) {
                return badInput_PunyStatus;
            }
            digit = decodeDigit_(input[in++]);
            if (digit >= base_PunyParam) {
                return badInput_PunyStatus;
            }
            if (digit > (UINT32_MAX - i) / w) {
                return overflow_PunyStatus;
            }
            i += digit * w;
            t = k <= bias ? tmin_PunyParam : k >= bias + tmax_PunyParam ? tmax_PunyParam : k - bias;
            if (digit < t) {
                break;
            }
            if (w > UINT32_MAX / (base_PunyParam - t)) {
                return overflow_PunyStatus;
            }
            w *= (base_PunyParam - t);
        }
        bias = adapt_(i - oldi, out + 1, oldi == 0);
        if (i / (out + 1) > UINT32_MAX - n) {
            return overflow_PunyStatus;
        }
        n += i / (out + 1);
        i %= (out + 1);
        if (out >= outMax) {
            return bigOutput_PunyStatus;
        }
        if (caseFlags) {
            memmove(caseFlags + i + 1, caseFlags + i, out - i);
            caseFlags[i] = isFlagged_(input[in - 1]);
        }
        memmove(output + i + 1, output + i, (out - i) * sizeof(*output));
        output[i++] = n;
    }

    *outputLength = out;
    return success_PunyStatus;
}

/*----------------------------------------------------------------------------------------------*/

iString *punyEncode_Rangecc(const iRangecc d) {
    iString *out = new_String();
    size_t inputLength = 0;
    uint32_t *input = u8_to_u32((const uint8_t *) d.start, size_Range(&d), NULL, &inputLength);
    resize_Block(&out->chars, inputLength * 2);
    for (;;) {
        uint32_t outputLength = size_Block(&out->chars);
        enum iPunyStatus result =
            encode_(inputLength, input, &outputLength, data_Block(&out->chars), NULL);
        if (result == bigOutput_PunyStatus) {
            resize_Block(&out->chars, size_Block(&out->chars) * 2);
            continue;
        }
        if (result == success_PunyStatus) {
            resize_Block(&out->chars, outputLength);
        }
        else {
            clear_String(out);
        }
        break;
    }
    free(input);
    return out;
}

iString *punyDecode_Rangecc(const iRangecc d) {
    uint32_t outputLength = size_Range(&d) * 2;
    uint32_t *out = malloc(outputLength * sizeof(uint32_t));
    iString *dec = NULL;
    for (;;) {
        enum iPunyStatus result = decode_(size_Range(&d), d.start, &outputLength, out, NULL);
        if (result == bigOutput_PunyStatus) {
            out = realloc(out, outputLength *= 2);
            continue;
        }
        if (result == success_PunyStatus) {
            dec = newUnicodeN_String(out, outputLength);
        }
        else {
            dec = new_String();
        }
        break;
    }
    free(out);
    return dec;
}
