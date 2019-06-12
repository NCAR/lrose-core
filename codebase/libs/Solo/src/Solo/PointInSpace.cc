#include <cstdio>
#include "Solo/PointInSpace.hh"

void PointInSpace::print() {

  //  stream, num, pisp)
  //    FILE *stream;
  //int num;
  //struct point_in_space *pisp;

  
  printf("bnd:%3ld; lon:%.4f; lat:%.4f; alt:%.4f; "
          , cell_num, longitude, latitude, altitude);
  printf("az:%.4f; el:%.4f; rng:%.4f;"
          , azimuth, elevation, range);
  printf("hdg:%.4f; tlt:%.4f; dft:%.4f; "
          , heading, tilt, drift);
  printf("\n");

}
