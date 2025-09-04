#include "mobile.h"

void safeAreaInsets_Mobile(float *left, float *top, float *right, float *bottom) {
    if (left)  *left  = 0.0f;
    if (top)   *top   = 0.0f;
    if (right) *right = 0.0f;
    if (bottom)*bottom= 0.0f;
}

