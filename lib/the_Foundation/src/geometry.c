#include "the_Foundation/geometry.h"

void init_Plane(iPlane *d, iFloat3 origin, iFloat3 normal) {
    d->origin = origin;
    d->normal = normalize_F3(normal);
}

float z_Plane(const iPlane *d, iFloat3 pos) {
    const float   dist = dot_F3(sub_F3(pos, d->origin), d->normal);
    const iFloat3 p    = sub_F3(pos, mulf_F3(d->normal, dist));
    return z_F3(p);
}
