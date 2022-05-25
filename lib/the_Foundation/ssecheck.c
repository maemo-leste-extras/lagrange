/* This is based on https://gist.github.com/hi2p-perim/7855506 */

#include <stdio.h>
#if defined (_MSC_VER) || defined (__MINGW64__)
#  include <intrin.h>
#else  
void __cpuid(int *cpuinfo, int info) {
    __asm__ __volatile__(
        "xchg %%ebx, %%edi;"
        "cpuid;"
        "xchg %%ebx, %%edi;"
        :"=a" (cpuinfo[0]), "=D" (cpuinfo[1]), "=c" (cpuinfo[2]), "=d" (cpuinfo[3])
        :"0" (info)
        );
}
#endif

int main(int argc, char *argv[]) {
    int cpuinfo[4];
    __cpuid(cpuinfo, 1);
    printf("%d\n", cpuinfo[2] & (1 << 19) ? 1 : 0); /* SSE 4.1 */
    return 0;
}
