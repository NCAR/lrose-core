
#include <climits>
#include "Rsl.hh"


Radar *RSL::new_radar(Radx::si32 nvolumes) {
  Radar *radar;
  
  radar = (Radar *) malloc(sizeof(Radar));
  if (radar != NULL) {
    radar->h.nvolumes = nvolumes;
    radar->v = (Volume **) malloc(sizeof(Volume *) * nvolumes);
  }
  return radar;
};

Volume *RSL::new_volume(Radx::si32 max_sweeps) {
  Volume *volume;
  
  volume = (Volume *) malloc(sizeof(Volume));
  if (volume != NULL) {
    volume->h.nsweeps = max_sweeps;
    volume->sweep = (Sweep **) malloc(sizeof(Sweep *) * max_sweeps);
  }
  return volume;
};

Sweep *RSL::new_sweep(Radx::si32 max_rays) {
  Sweep *sweep;
  
  sweep = (Sweep *) malloc(sizeof(Sweep));
  if (sweep != NULL) {
    sweep->h.nrays = max_rays;
    sweep->ray = (Ray **) malloc(sizeof(Ray *) * max_rays);
  }
  return sweep;
};

Ray *RSL::new_ray(Radx::si32 max_bins) {
  Ray *ray;
  
  ray = (Ray *) malloc(sizeof(Ray));
  if (ray != NULL) {
    ray->h.nbins = max_bins;
    ray->range = (Range *) malloc(sizeof(Range) * max_bins);
  }
  return ray;
};

void RSL::free_radar(Radar *radar) {
  Volume **ptr = radar->v;
  for (Radx::si32 i=0; i<radar->h.nvolumes; i++) {
    free(*ptr);
    ptr++;
  } 
  free(radar->v);
  free(radar);
}

void RSL::free_sweep(Sweep *sweep) {
  Ray **ptr = sweep->ray;
  for (Radx::si32 i=0; i<sweep->h.nrays; i++) {
    free(*ptr);
    ptr++;
  } 
  free(sweep->ray);
  free(sweep);
}

void RSL::free_volume(Volume *volume) {
  Sweep **ptr = volume->sweep;
  for (Radx::si32 i=0; i<volume->h.nsweeps; i++) {
    free(*ptr);
    ptr++;
  } 
  free(volume->sweep);
  free(volume);
}

/*
void RSL_free_sweep(Sweep *sweep) {
  Ray *ptr = sweep->ray;
  for (Radx::si32 i=0; i<sweep->h.nrays; i++) {
    free(*ptr);
    ptr++;
  } 
  free(sweep->ray);
  free(sweep);
}
*/

// TODO: write some unit tests for these

Volume *RSL::copy_volume(Volume *v) {

  Volume *the_copy;
  the_copy = RSL::new_volume(v->h.nsweeps);

  // allocate and copy the string for type_str
  size_t len = sizeof(v->h.type_str);
  the_copy->h.type_str = (char *) malloc(len);
  memcpy(the_copy->h.type_str, v->h.type_str, len);
  the_copy->h.calibr_const = v->h.calibr_const;
  // how to copy the data conversion functions?
  
  // does this do a deep copy? does it copy all the rays and data too?
  memcpy(the_copy->sweep, v->sweep, sizeof(Sweep) * v->h.nsweeps);
  return the_copy;
}


/* Internal storage conversion functions. These may be any conversion and
 * may be dynamically defined; based on the input data conversion.
 */

// TODO: write some unit tests for these
// convert from 4 bytes <==> 2 bytes
Radx::fl32 RSL::DZ_F(Range x) { 
  return (Radx::fl32) x; 
}

Radx::fl32 RSL::VR_F(Range x) { 
  return (Radx::fl32) x; 
}

// TODO: do I need to use the scale and bias to convert these?

Range RSL::DZ_INVF(Radx::fl32 x) 
{ 
  // check for overflow or underflow
  if (x > UINT_MAX)
    throw "value too large for conversion to 2-byte unsigned int"; 
  if (x < 0)
    throw "value too small for conversion to 2-byte unsigned int"; 
  return (Range) x; 
}


Range RSL::VR_INVF(Radx::fl32 x)
{ 
  // check for overflow or underflow
  if (x > UINT_MAX)
    throw "value too large for conversion to 2-byte unsigned int"; 
  if (x < 0) 
    throw "value too small for conversion to 2-byte unsigned int";
  return (Range) x; 
}

