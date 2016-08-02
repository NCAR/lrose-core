//
//
//
//

#ifndef VECTORDEFS_H
#define VECTORDEFS_H

#include <vector>


typedef std::vector<int>                RayIntegers;
typedef std::vector<RayIntegers *>      FieldRayIntegers;
typedef std::vector<FieldRayIntegers *> VolumeIntegers;

typedef std::vector<double>             RayDoubles;
typedef std::vector<RayDoubles *>       FieldRayDoubles;
typedef std::vector<FieldRayDoubles *>  VolumeDoubles;

#endif // VECTORDEFS_H
