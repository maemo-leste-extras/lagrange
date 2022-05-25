/**
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

#include <the_Foundation/defs.h>
#include <the_Foundation/math.h>
#include <the_Foundation/time.h>
#include <the_Foundation/fixed2.h>

static void printNum(float n) {
    if (n == 0.f) {
        printf("      .      ");
    }
    else {
        printf("%13f", n);
    }
}

static void printi2(const char *msg, const iInt2 v) {
    printf("%16s: (%d, %d)\n", msg, v.x, v.y);
}

static void printFixed(const iFixed x) {
    uint32_t frac = x.comp.sign ? iFixedUnit - x.comp.frac : x.comp.frac;
    uint64_t wnum = x.comp.sign ? iFixedMaxWNum - x.comp.wnum : x.comp.wnum;
    if (frac == iFixedUnit) {
        frac = 0;
        wnum++;
    }
    printf("%c%llu.%05d",
           x.comp.sign ? '-' : '+',
           wnum,
           (int) (f32_Fixed(init_Fixed(frac)) * 100000));
}

static void printx(const char *msg, const iFixed x) {
    printf("%16s: ", msg);
    printFixed(x);
    printf("\n");
}

static void printx2(const char *msg, const iFixed2 v) {
    printf("%16s: ( ", msg);
    printFixed(v.x);
    printf("  ");
    printFixed(v.y);
    printf(" )\n");
}

static void printv(const char *msg, iFloat4 v) {
    const iFloatVec4 vec = values_F4(v);
    printf("%9s: ( ", msg);
    for (int i = 0; i < 4; ++i) printNum(vec.v[i]);
    printf(" )\n");
}

static void printv3(const char *msg, iFloat3 v) {
    const iFloatVec3 vec = values_F3(v);
    printf("%9s: ( ", msg);
    for (int i = 0; i < 3; ++i) printNum(vec.v[i]);
    printf(" )\n");
}

static void printMat3(const char *msg, const iMat3 *m) {
    float vals[9];
    store_Mat3(m, vals);
    printf("%s: [", msg);
    for (int i = 0; i < 9; i += 3) {
        printf("\n    ");
        for (int j = 0; j < 3; ++j) printNum(vals[i+j]);
    }
    puts(" ]");
}

static void printMat(const char *msg, const iMat4 *m) {
    float vals[16];
    store_Mat4(m, vals);
    printf("%s: [", msg);
    for (int row = 0; row < 4; ++row) {
        printf("\n    ");
        for (int col = 0; col < 4; ++col) printNum(vals[col * 4 + row]);
    }
    puts(" ]");
}

#define print_(msg, val) _Generic(val, \
    iFloat4:       printv, \
    iFloat3:       printv3, \
    iFixed:        printx, \
    iFixed2:       printx2, \
    iInt2:         printi2, \
    iMat3 *:       printMat3, \
    const iMat3 *: printMat3, \
    iMat4 *:       printMat, \
    const iMat4 *: printMat)(msg, val)

int main(int argc, char **argv) {
    init_Foundation();
    iUnused(argc, argv);
    /* Scalar operations. */ {
        printf("min:%f max:%f clamp:%f %f %f\n",
                iMinf(3.f, 5.f),
                iMaxf(3.f, 5.f),
                iClampf(1, 3, 5), iClampf(7, 3, 5), iClampf(4, 3, 5));
    }
    /* Basic vectoring. */ {
        print_("zero", zero_F4());
        print_("init4", init_F4(1, 2, 3, 4)); {
            float f[4];
            store_F4(init_F4(1, 2, 3, 4), f);
            printf("stored: %f %f %f %f\n", f[0], f[1], f[2], f[3]);
        }
        print_("zxy", zxy_F4(init_F4(1, 2, 3, 4)));
        iFloat4 a = init_F4(1.5, -2, 3.5, -4);
        iFloat4 b = init_F4(iRandomf(), iRandomf(), iRandomf(), 1);
        setW_F4(&b, 100.5f);
        print_("a", a);
        print_("neg(a)", neg_F4(a));
        print_("abs(a)", abs_F4(a));
        print_("b", b);
        print_("a+b", add_F4(a, b));
        printf("Length: %f %f %f\n",
               length_F3(init_F3(-2, 0, 0)),
               length_F3(init_F3(0, -2, 0)),
               length_F3(init_F3(0, 0, -2)));
        printf("Sum: F3: %f F4: %f\n", sum_F3(init_F3(1, 2, 3)), sum_F4(init_F4(1, 2, 3, 4)));
        print_("init3", init_F3(2, 3, 4)); {
            iFloat3 v3 = init_F3(1, 1, 1);
            setX_F3(&v3, 2);
            print_("set x", v3);
            setY_F3(&v3, 3);
            print_("set y", v3);
            setZ_F3(&v3, 4);
            print_("set z", v3);
        }
        for (int i = 0; i <= 10; ++i) {
            printf("%2i: ", i);
            print_("mix", mix_F4(a, b, i/10.f));
        }
    }
    /* 3x3 matrix. */ {
        iMat3 mat3;
        init_Mat3(&mat3);
        print_("mat3", &mat3);
    }
    /* Fixed-point numbers. */ {
        puts("Fixed-point:");
        print_("0.65f", initf_Fixed(0.65f));
        print_("-0.25f", initf_Fixed(-0.25f));
        print_("-1", initi_Fixed(-1));
        print_("-2", initi_Fixed(-2));
        print_("Pi", initd_Fixed(iMathPi));
        print_("1<<32 + half", add_Fixed(init_Fixed(1LL << 32 << iFixedFracBits), half_Fixed()));
        print_("mul 0.25 * -4", muli_Fixed(initd_Fixed(0.25), -4));
        print_("div -5 / -0.5", div_Fixed(initi_Fixed(-5), neg_Fixed(half_Fixed())));
        print_("random -10...10", mix_Fixed(initi_Fixed(-10), initi_Fixed(10), random_Fixed()));
    }
    /* Integer 2D vectors. */ {
        puts("Int2 vectors:");
        iInt2 v = init_I2(7, 19);
        print_("v", v);
        printf("%16s: %f\n", "length_I2", length_I2(v));
    }
    /* Fixed-point 2D vectors. */ {
        puts("Fixed-point vectors:");
        iFixed2 v = initi_X2(1, 0);
        print_("(1,0)", v);
        print_("add one", addv_X2(&v, init1_X2(one_Fixed())));
        print_("halved", mulv_X2(&v, divi_X2(one_X2(), 2)));
        print_("length", length_X2(v));
        printf("  length (float): %f\n", lengthf_X2(v));
        print_("random", mix_X2(initi_X2(-1, 100), initi_X2(1, -100), random_Fixed()));
    }
    /* Matrices. */ {
        printf("--------------------------------------------------------\n");
        printf("dot3: %f\n", dot_F3(init_F3(1, 2, 3), init_F3(3, 4, 2)));

        iMat4 mat, b, c;
        init_Mat4(&mat);
        print_("identity", &mat);
        initScale_Mat4(&b, init1_F3(3));
        print_("3x", &b);
        mul_Mat4(&mat, &b);
        print_("mult", &mat);
        copy_Mat4(&c, &mat);
        scalef_Mat4(&c, .1f);
        print_("scaled", &c);

        iMat4 s;
        initScale_Mat4(&s, init_F3(4, 3, 2));
        print_("scale", &s);

        iMat4 inverseScale;
        inverse_Mat4(&s, &inverseScale);
        print_("inv scale", &inverseScale);

        printv("vec mul", mulF4_Mat4(&s, init_F4(10, 20, 30, 1)));

        iMat4 t;
        initTranslate_Mat4(&t, init_F3(1, 2, 3));
        print_("xlat", &t);
        //printv3("vec trl", mulF3_Mat4(&t, init_F3(14, 13, 12)));
        print_("vec trl", mulF3_Mat4(&t, init_F3(iRandomf(), iRandomf(), iRandomf())));

        iMat4 s_t = s;
        iMat4 t_s = t;
        mul_Mat4(&s_t, &t);
        mul_Mat4(&t_s, &s);
        print_("s*t", &s_t);
        print_("t*s", &t_s);

        print_("s*t * (10,20,30)", mulF3_Mat4(&s_t, init_F3(10, 20, 30)));
        print_("t*s * (10,20,30)", mulF3_Mat4(&t_s, init_F3(10, 20, 30)));

        /* Rotations. */
        for (int ang = 0; ang <= 360; ang += 45) {
            iMat4 rot;
            initRotate_Mat4(&rot, init_F3(0, 1, 0), ang);
            iFloat3 rp = mulF3_Mat4(&rot, init_F3(2, 0, 0));
            print_("rotated", rp);
            printf("len: %f\n", length_F3(rp));
        }

#define REPS 3000000
        iFloat3 *res = malloc(sizeof(iFloat3) * REPS);
        const iTime start = now_Time();
        for (int rep = 0; rep < REPS; ++rep) {
            /* Perf test. */ {
                iMat4 rot;
                initRotate_Mat4(&rot, init_F3(0, 1, 0), -rep);
                //printMat("rot", &rot);
                res[rep] = /*printv3("rotated", */mulF3_Mat4(&rot, init_F3(0, 0, rep));
            }
        }
        printf("Rotation: elapsed %lf seconds\n", elapsedSeconds_Time(&start));
        free(res);
    }
    /* Inversion. */ {
        iMat4 matrix;
        const float rowMajorValues[16] = {
             0.257f,  0.504f,  0.098f, 16,
             0.439f, -0.368f, -0.071f, 128,
            -0.148f, -0.291f,  0.439f, 128,
            0, 0, 0, 1
        };
        load_Mat4(&matrix, rowMajorValues);
        transpose_Mat4(&matrix);
        print_("RGB-to-YUV", &matrix);
        iMat4 inverse;
        inverse_Mat4(&matrix, &inverse);
        print_("YUV-to-RGB", &inverse);
        //printv3("yuv", mulF3_Mat4(&matrix, init_F3(0, 0, 255)));
        iFloat3 color = init_F3(101, 123, 148);
        print_("original RGB", color);
        print_("YUV", mulF3_Mat4(&matrix, color));
        print_("back to RGB", mulF3_Mat4(&inverse, mulF3_Mat4(&matrix, color)));
        return 123;
    }
}
