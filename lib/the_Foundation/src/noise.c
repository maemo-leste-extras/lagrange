#include "the_Foundation/noise.h"
#include "the_Foundation/array.h"
#include "the_Foundation/math.h"
#include "the_Foundation/geometry.h"
#include "the_Foundation/stream.h"

struct Impl_Noise {
    iInt2    size;
    float    scale; // normalizing output values
    iFloat3 *gradients;
};

iDefineTypeConstructionArgs(Noise, (iInt2 size), size)

iLocalDef iFloat3 *gradient_Noise_(const iNoise *d, const iInt2 pos) {
    return d->gradients + (d->size.x * pos.y + pos.x);
}

void init_Noise(iNoise *d, iInt2 size) {
    d->size = add_I2(size, one_I2()); // gradients at cell corners
    d->scale = 1.45f;
    d->gradients = malloc(sizeof(iFloat3) * (size_t) prod_I2(d->size));
    for (int i = 0; i < prod_I2(d->size); ++i) {
        const float angle = iRandomf() * iMathPif * 2.f;
        d->gradients[i] = init_F3(cosf(angle), sinf(angle), 0.f);
    }
}

void deinit_Noise(iNoise *d) {
    free(d->gradients);
}

void serialize_Noise(const iNoise *d, iStream *outs) {
    writeInt2_Stream(outs, d->size);
    writef_Stream(outs, d->scale);
    for (int i = 0; i < prod_I2(d->size); ++i) {
        writeFloat3_Stream(outs, d->gradients[i]);
    }
}

void deserialize_Noise(iNoise *d, iStream *ins) {
    d->size = readInt2_Stream(ins);
    d->scale = readf_Stream(ins);
    d->gradients = realloc(d->gradients, sizeof(iFloat3) * (size_t) prod_I2(d->size));
    for (int i = 0; i < prod_I2(d->size); ++i) {
        d->gradients[i] = readFloat3_Stream(ins);
    }
}

iLocalDef float dotGradient_Noise_(const iNoise *d, const int x, int y, const iFloat3 b) {
    return dot_F3(sub_F3(b, initi_F3(x, y, 0)), *gradient_Noise_(d, init_I2(x, y)));
}

iLocalDef float hermite_(float a, float b, float w) {
    w = iClamp(w, 0, 1);
    return a + (b - a) * (w * w * (3 - 2 * w));
}

iInt2 size_Noise(const iNoise *d) {
    return sub_I2(d->size, one_I2());
}

float eval_Noise(const iNoise *d, float normX, float normY) {
    const float x = normX * (d->size.x - 1);
    const float y = normY * (d->size.y - 1);
    const iInt2 c0 = init_I2((int) x, (int) y);
    const iInt2 c1 = add_I2(c0, init_I2(1, 1));
    if (any_Boolv(less_I2(c0, zero_I2())) || any_Boolv(greaterEqual_I2(c1, d->size))) {
        return 0.f;
    }
    const iFloat3 pos = init_F3(x, y, 0.f);
    const float s0 = hermite_(
        dotGradient_Noise_(d, c0.x, c0.y, pos), dotGradient_Noise_(d, c1.x, c0.y, pos), x - c0.x);
    const float s1 = hermite_(
        dotGradient_Noise_(d, c0.x, c1.y, pos), dotGradient_Noise_(d, c1.x, c1.y, pos), x - c0.x);
    return hermite_(s0, s1, y - c0.y) * d->scale;
}

/*-----------------------------------------------------------------------------------------------*/

iDeclareType(CombinedNoisePart)

struct Impl_CombinedNoisePart {
    float weight;
    float offset;
    iNoise noise;
};

struct Impl_CombinedNoise {
    iArray parts;
    iArray offsets; // iFloat3
};

#if 0
iLocalDef const iPlane *quadrantPlane_CombinedNoise_(const iCombinedNoise *d, const iFloat3 normPos) {
    int index = 0;
    if (x_F3(normPos) >= .5f) index |= 1;
    if (y_F3(normPos) >= .5f) index |= 2;
    return d->planes + index;
}
#endif

iDefineTypeConstructionArgs(CombinedNoise,
                            (const iNoiseComponent *components, size_t count),
                            components, count)

void init_CombinedNoise(iCombinedNoise *d, const iNoiseComponent *components, size_t count) {
    init_Array(&d->parts, sizeof(iCombinedNoisePart));
    for (size_t i = 0; i < count; ++i) {
        iCombinedNoisePart part = { .weight = components[i].weight,
                                    .offset = components[i].offset };
        init_Noise(&part.noise, components[i].size);
        pushBack_Array(&d->parts, &part);
    }
//    for (size_t i = 0; i < iElemCount(d->planes); ++i) {
//        init_Plane(d->planes + i, zero_F3(), init_F3(0.f, 0.f, 1.f));
//    }
    init_Array(&d->offsets, sizeof(iFloat3));
}

void deinit_CombinedNoise(iCombinedNoise *d) {
    iForEach(Array, i, &d->parts) {
        deinit_Noise(&((iCombinedNoisePart *) i.value)->noise);
    }
    deinit_Array(&d->parts);
    deinit_Array(&d->offsets);
}

void serialize_CombinedNoise(const iCombinedNoise *d, iStream *outs) {
    writeU16_Stream(outs, (uint16_t) size_Array(&d->parts));
    iConstForEach(Array, i, &d->parts) {
        const iCombinedNoisePart *part = i.value;
        writef_Stream(outs, part->weight);
        writef_Stream(outs, part->offset);
        serialize_Noise(&part->noise, outs);
    }
    writeU16_Stream(outs, (uint16_t) size_Array(&d->offsets));
    iConstForEach(Array, j, &d->offsets) {
        writeFloat3_Stream(outs, *(const iFloat3 *) j.value);
    }
}

void deserialize_CombinedNoise(iCombinedNoise *d, iStream *ins) {
    deinit_CombinedNoise(d);
    init_CombinedNoise(d, NULL, 0);
    const size_t numParts = readU16_Stream(ins);
    resize_Array(&d->parts, numParts);
    iForEach(Array, i, &d->parts) {
        iCombinedNoisePart *part = (iCombinedNoisePart *) i.value;
        part->weight = readf_Stream(ins);
        part->offset = readf_Stream(ins);
        init_Noise(&part->noise, zero_I2());
        deserialize_Noise(&part->noise, ins);
    }
    const size_t numOffsets = readU16_Stream(ins);
    resize_Array(&d->offsets, numOffsets);
    iForEach(Array, j, &d->offsets) {
        *((iFloat3 *) j.value) = readFloat3_Stream(ins);
    }
}

#if 0
void setQuadrantPlane_CombinedNoise(iCombinedNoise *d, size_t quadrant, iFloat3 origin, iFloat3 normal) {
    if (quadrant >= iElemCount(d->planes)) {
        for (quadrant = 0; quadrant < iElemCount(d->planes); ++quadrant) {
            setQuadrantPlane_CombinedNoise(d, quadrant, origin, normal);
        }
    }
    else {
        init_Plane(d->planes + quadrant, origin, normal);
    }
}
#endif

static float weightedOffset_CombinedNoise_(const iCombinedNoise *d, float x, float y) {
    if (isEmpty_Array(&d->offsets)) {
        return 0.f;
    }
    iDeclareType(Nearest);
    struct Impl_Nearest {
        size_t index;
        float dist;
    };
    iNearest nearest[] = {
        { iInvalidPos, 0.f }, { iInvalidPos, 0.f }, { iInvalidPos, 0.f }, { iInvalidPos, 0.f }
    };
    const iFloat3 pos = init_F3(x, y, 0.f);
    iConstForEach(Array, i, &d->offsets) {
        const iFloat3 offPos = *(const iFloat3 *) i.value;
        float dist = length_F3(sub_F3(init_F3(x_F3(offPos), y_F3(offPos), 0.f), pos));
        /* Is this one of the nearest ones? */
        iForIndices(k, nearest) {
            iNearest *near = nearest + k;
            if (near->index == iInvalidPos || dist < near->dist) {
                near->index = index_ArrayConstIterator(&i);
                near->dist = dist;
                break;
            }
        }
    }
    float maxDist = 0.f;
    iForIndices(k, nearest) {
        maxDist = iMax(nearest[k].dist, maxDist);
    }
    float weights = 0.f;
    float offset = 0.f;
    for (size_t k = 0; k < iElemCount(nearest); ++k) {
        const iNearest *near = nearest + k;
        if (near->index != iInvalidPos) {
            const iFloat3 offPos = *(const iFloat3 *) constAt_Array(&d->offsets, near->index);
            const float weight = (maxDist - near->dist) / maxDist;
            offset += z_F3(offPos) * weight;
            weights += weight;
        }
    }
    if (weights > 0.f) {
        offset /= weights;
    }
    return offset;
}

float eval_CombinedNoise(const iCombinedNoise *d, float normX, float normY) {
    float value = 0.f;
    iConstForEach(Array, i, &d->parts) {
        const iCombinedNoisePart *part = i.value;
        value += part->weight * eval_Noise(&part->noise, normX, normY) + part->offset;
    }
    return value + weightedOffset_CombinedNoise_(d, normX, normY);
}

iFloat3 randomCoord_CombinedNoise(const iCombinedNoise *d, iBool (*rangeCheck)(float)) {
    const int maxAttempts = 1000;
    for (int i = 0; i < maxAttempts; ++i) {
        iFloat3 pos = init_F3(iRandomf(), iRandomf(), 0.f);
        if (rangeCheck(eval_CombinedNoise(d, x_F3(pos), y_F3(pos)))) {
            return pos;
        }
    }
    return init1_F3(-1.f);
}

void setOffset_CombinedNoise(iCombinedNoise *d, size_t index, float offset) {
    ((iCombinedNoisePart *) at_Array(&d->parts, index))->offset = offset;
}

void setPointOffset_CombinedNoise(iCombinedNoise *d, float normX, float normY, float offset) {
    const iFloat3 off = init_F3(normX, normY, offset);
    pushBack_Array(&d->offsets, &off);
}
