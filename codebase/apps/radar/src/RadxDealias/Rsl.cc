#include <iostream>
#include <climits>
#include "Rsl.hh"


Radar *Rsl::new_radar(Radx::si32 nvolumes) {
  Radar *radar;
  
  radar = (Radar *) malloc(sizeof(Radar));
  if (radar != NULL) {
    radar->h.nvolumes = nvolumes;
    radar->v = (Volume **) malloc(sizeof(Volume *) * nvolumes);
  }
  if (radar->v == NULL) 
    throw std::bad_alloc(); // "Memory allocation failed for Radar");
  for (int i=0; i<nvolumes; i++) 
    radar->v[i] = NULL;
  
  return radar;
};

Volume *Rsl::new_volume(Radx::si32 max_sweeps) {
  Volume *volume;
  
  volume = (Volume *) malloc(sizeof(Volume));
  if (volume != NULL) {
    volume->h.nsweeps = max_sweeps;
    volume->sweep = (Sweep **) malloc(sizeof(Sweep *) * max_sweeps);
  }
  if (volume->sweep == NULL) 
    throw std::bad_alloc(); // "Memory allocation failed for Volume");
  for (int i=0; i<max_sweeps; i++) 
    volume->sweep[i] = NULL;

  volume->h.type_str = NULL;

  return volume;
};

Sweep *Rsl::new_sweep(Radx::si32 max_rays) {
  Sweep *sweep;
  
  sweep = (Sweep *) malloc(sizeof(Sweep));
  if (sweep != NULL) {
    sweep->h.nrays = max_rays;
    sweep->ray = (Ray **) malloc(sizeof(Ray *) * max_rays);
  }
  if (sweep->ray == NULL) 
    throw std::bad_alloc(); // "Memory allocation failed for Sweep");
  for (int i=0; i<max_rays; i++) 
    sweep->ray[i] = NULL;

  return sweep;
};

Ray *Rsl::new_ray(Radx::si32 max_bins) {
  Ray *ray;
  
  ray = (Ray *) malloc(sizeof(Ray));
  if (ray != NULL) {
    ray->h.nbins = max_bins;
    // don't allocate memory, we are just going to use pointers to data
    //ray->range = (Range *) malloc(sizeof(Range) * max_bins);
  }
  return ray;
};

// the free methods, free the base structure AND
// every structure embedded in it.  So, free_radar
// frees the Radar structure, the Volumes, the Sweeps,
// the Rays, AND the bins.
//
void Rsl::free_radar(Radar *radar) {
  if (radar == NULL) return;

  Volume **ptr = radar->v;
  for (Radx::si32 i=0; i<radar->h.nvolumes; i++) {
    free_volume(*ptr);
    ptr++;
  } 
  free(radar->v);
  free(radar);
}

void Rsl::free_volume(Volume *volume) {
  if (volume == NULL) return;

  Sweep **ptr = volume->sweep;
  for (Radx::si32 i=0; i<volume->h.nsweeps; i++) {
    free_sweep(*ptr);
    ptr++;
  } 
  free(volume->sweep);
  char *type_str = volume->h.type_str;
  if (type_str != NULL) 
    free(type_str);
  free(volume);
}

void Rsl::free_sweep(Sweep *sweep) {
  if (sweep == NULL) return;

  Ray **ptr = sweep->ray;
  for (Radx::si32 i=0; i<sweep->h.nrays; i++) {
    free_ray(*ptr);
    ptr++;
  } 
  free(sweep->ray);
  free(sweep);
}


void Rsl::free_ray(Ray *ray) {
  if (ray == NULL) return;

  Range *ptr = ray->range;
  free(ptr);
  free(ray);
}


// TODO: write some unit tests for these

Volume *Rsl::copy_volume(Volume *v) {

  if (v == NULL) return NULL;

  Volume *the_copy;
  the_copy = Rsl::new_volume(v->h.nsweeps);

  // TODO: fix this up ...
  // allocate and copy the string for type_str
  if (v->h.type_str != NULL) {
    size_t len = sizeof(v->h.type_str);
    the_copy->h.type_str = (char *) malloc(len);
    memcpy(the_copy->h.type_str, v->h.type_str, len);
    the_copy->h.calibr_const = v->h.calibr_const;
  }
 

  // how to copy the data conversion functions?
  
  // does this do a deep copy? does it copy all the rays and data too?
  memcpy(the_copy->sweep, v->sweep, sizeof(Sweep) * v->h.nsweeps);
  return the_copy;
}


/* Internal storage conversion functions. These may be any conversion and
 * may be dynamically defined; based on the input data conversion.
 */

// convert from 4 bytes <==> 2 bytes
Radx::fl32 Rsl::DZ_F(Range x) { 
  return (Radx::fl32) x; 
}

Radx::fl32 Rsl::VR_F(Range x) { 
  return (Radx::fl32) x; 
}

// TODO: do I need to use the scale and bias to convert these?

Range Rsl::DZ_INVF(Radx::fl32 x) 
{ 
  // check for overflow or underflow
  if (x > UINT_MAX)
    throw std::out_of_range("value too large for conversion to 2-byte unsigned int"); 
  if (x < 0)
    throw std::out_of_range("value too small for conversion to 2-byte unsigned int"); 
  return (Range) x; 
}


Range Rsl::VR_INVF(Radx::fl32 x)
{ 
  // check for overflow or underflow
  if (x > UINT_MAX)
    throw std::out_of_range("value too large for conversion to 2-byte unsigned int"); 
  if (x < 0) 
    throw std::out_of_range("value too small for conversion to 2-byte unsigned int");
  return (Range) x; 
}

void Rsl::print_ray_header(Ray_header header) {
  
  cout << "Ray header " << endl;
  cout << " --------" << endl;
  cout << "    azimuth=" << header.azimuth << endl;
  cout << "       elev=" << header.elev << endl;
  cout << "range bin 1=" << header.range_bin1 << endl;
  cout << "  gate size=" << header.gate_size << endl;
  cout << "       bias=" << header.bias << endl;
  cout << "      scale=" << header.scale << endl;
  cout << "    nyq vel=" << header.nyq_vel << endl;
  cout << " --------" << endl;
}


// print the Ray 
void Rsl::print_ray(Ray *ray) 
{

  if (ray == NULL) 
    cout << "Ray is NULL" << endl;
  else {
    // print the header
    print_ray_header(ray->h);

    // print the data
    if (ray->range != NULL) {
      Range *bin = ray->range;
      for (int i = 0; i < ray->h.nbins; i++) {
	cout << bin[i] << " ";
      }
      cout << endl;
    }
  }
}


// print the Sweep and walk down the pointers to Rays and print the contents
void Rsl::print_sweep(Sweep *sweep) 
{

  if (sweep == NULL) 
    cout << "Sweep is NULL" << endl;
  else {
    // print the sweep header
    //print_sweep_header(sweep->h);

    // print each ray
    if (sweep->ray != NULL) {
      Ray **ray = sweep->ray;
      for (int i = 0; i < sweep->h.nrays; i++) {
	print_ray(ray[i]);
      }
    }
  }
}



// print the Volume and walk down the pointers to Rays and print the contents
void Rsl::print_volume(Volume *volume) 
{

  cout << " ==============" << endl;
  if (volume == NULL) 
    cout << "Volume is NULL" << endl;
  else {
    // print the volume header
    //print_volume_header(volume->h);

    // print each sweep
    if (volume->sweep != NULL) {
      Sweep **sweep = volume->sweep;
      for (int i = 0; i < volume->h.nsweeps; i++) {
	print_sweep(sweep[i]);
      }
    }
  }
  cout << " ================" << endl;

}

