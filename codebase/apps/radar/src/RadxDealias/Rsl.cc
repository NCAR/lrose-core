// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1990 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Boulder, Colorado, USA
// ** BSD licence applies - redistribution and use in source and binary
// ** forms, with or without modification, are permitted provided that
// ** the following conditions are met:
// ** 1) If the software is modified to produce derivative works,
// ** such modified software should be clearly marked, so as not
// ** to confuse it with the version available from UCAR.
// ** 2) Redistributions of source code must retain the above copyright
// ** notice, this list of conditions and the following disclaimer.
// ** 3) Redistributions in binary form must reproduce the above copyright
// ** notice, this list of conditions and the following disclaimer in the
// ** documentation and/or other materials provided with the distribution.
// ** 4) Neither the name of UCAR nor the names of its contributors,
// ** if any, may be used to endorse or promote products derived from
// ** this software without specific prior written permission.
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
/////////////////////////////////////////////////////////
//
// Rsl.cc: methods of Rsl class; based on trmm RSL code
//
/////////////////////////////////////////////////////////

#include <iostream>
#include <climits>
#include <cmath>
#include <algorithm>
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
}

Volume *Rsl::new_volume(Radx::si32 max_sweeps) {
  Volume *volume;

  volume = (Volume *) malloc(sizeof(Volume));
  if (volume != NULL) {
    volume->h.nsweeps = max_sweeps;
    volume->sweep = (Sweep **) malloc(sizeof(Sweep *) * max_sweeps);
  }
  if (volume->sweep == NULL)
    throw std::bad_alloc(); // "Memory allocation failed for Volume");
  Sweep **sweeps = volume->sweep;
  for (int i=0; i<max_sweeps; i++)
    sweeps[i] = NULL;

  volume->h.type_str = NULL;

  return volume;
}

Sweep *Rsl::new_sweep(Radx::si32 max_rays) {
  Sweep *sweep;

  sweep = (Sweep *) malloc(sizeof(Sweep));
  if (sweep != NULL) {
    sweep->h.nrays = max_rays;
    sweep->h.maxBinsInSweep = 0;
    sweep->ray = (Ray **) malloc(sizeof(Ray *) * max_rays);
  }
  if (sweep->ray == NULL)
    throw std::bad_alloc(); // "Memory allocation failed for Sweep");
  Ray **rays = sweep->ray;
  for (int i=0; i<max_rays; i++)
    rays[i] = NULL;

  return sweep;
}

Ray *Rsl::new_ray(Radx::si32 max_bins) {
  Ray *ray;

  ray = (Ray *) malloc(sizeof(Ray));
  if (ray != NULL) {
    ray->h.nbins = max_bins;
    // don't allocate memory, we are just going to use pointers to data
    ray->h.binDataAllocated = false;
  }

  return ray;
}

// the free methods, free the base structure AND
// every structure embedded in it.  So, free_radar
// frees the Radar structure, the Volumes, the Sweeps,
// the Rays, AND the bins.
//
void Rsl::free_radar(Radar *radar) {
  if (radar == NULL) return;

  Volume **volumes = radar->v;
  for (Radx::si32 i=0; i<radar->h.nvolumes; i++) {
    free_volume(volumes[i]);
  }
  free(radar->v);
  free(radar);
}

void Rsl::free_volume(Volume *volume) {
  if (volume == NULL) return;

  Sweep **sweeps = volume->sweep;
  for (Radx::si32 i=0; i<volume->h.nsweeps; i++) {
    free_sweep(sweeps[i]);
  }
  free(volume->sweep);
  char *type_str = volume->h.type_str;
  if (type_str != NULL)
    free(type_str);
  free(volume);
}

void Rsl::free_sweep(Sweep *sweep) {
  if (sweep == NULL) return;

  Ray **rays = sweep->ray;
  for (Radx::si32 i=0; i<sweep->h.nrays; i++) {
    free_ray(rays[i]);
  }
  free(sweep->ray);
  free(sweep);
}


void Rsl::free_ray(Ray *ray) {
  if (ray == NULL) return;

  if (ray->h.binDataAllocated) {
    free(ray->range);
  }
  // else
  // don't free the data because it is used by RadxVol
  //Range *ptr = ray->range;

  free(ray);
}


Ray *Rsl::copy_ray(Ray *ray) {

  if (ray == NULL) return NULL;

  Ray *rayCopy;
  int nbins = ray->h.nbins;
  rayCopy = Rsl::new_ray(nbins);
  //memcpy(&(rayCopy->h), &(ray->h), sizeof(Ray_header));
  rayCopy->h.nbins = ray->h.nbins;


  rayCopy->h.azimuth = ray->h.azimuth;
  rayCopy->h.elev = ray->h.elev;
  rayCopy->h.nyq_vel = ray->h.nyq_vel;
  rayCopy->h.gate_size = ray->h.gate_size;;
  rayCopy->h.range_bin1 = ray->h.range_bin1;
  rayCopy->h.bias = ray->h.bias;
  rayCopy->h.scale = ray->h.scale;
  rayCopy->h.alt = ray->h.alt;

  // copy the bin data
  /*
  Range *binData = (Range *) malloc(sizeof(Range) * nbins);

  memcpy(binData, ray->range, ray->h.nbins * sizeof(Range));
  rayCopy->range = binData;
  */

  rayCopy->range = (Range *) malloc(sizeof(Range) * nbins);
  rayCopy->h.binDataAllocated = true;

  for (int i=0; i<nbins; i++)
    rayCopy->range[i] = ray->range[i];

  return rayCopy;
}



Sweep *Rsl::copy_sweep(Sweep *sweep) {

  if (sweep == NULL) return NULL;

  Sweep *sweepCopy;
  sweepCopy = Rsl::new_sweep(sweep->h.nrays);
  //memcpy(&(sweepCopy->h), &(sweep->h), sizeof(Sweep_header));
  sweepCopy->h.nrays = sweep->h.nrays;
  sweepCopy->h.maxBinsInSweep = sweep->h.maxBinsInSweep;

  // copy the rays
  for (int i=0; i<sweep->h.nrays; i++) {
    //Ray **rays = sweep->ray;
    //Ray **copyOfRays = sweepCopy->ray;
    //Ray *rayCopy = Rsl::copy_ray(rays[i]);
    //copyOfRays[i] = rayCopy;
    //sweepCopy->ray[i] = rayCopy;  // new
    sweepCopy->ray[i] = Rsl::copy_ray(sweep->ray[i]);
  }

  // does this do a deep copy? does it copy all the rays and data, too? Yes.

  return sweepCopy;
}




Volume *Rsl::copy_volume(Volume *v, bool debug) {

  if (v == NULL) return NULL;

  Volume *vcopy;
  vcopy = Rsl::new_volume(v->h.nsweeps);

  // allocate and copy the string for type_str
  if (v->h.type_str != NULL) {
    size_t len = sizeof(v->h.type_str);
    vcopy->h.type_str = (char *) malloc(len);
    //memcpy(vcopy->h.type_str, v->h.type_str, len);
  }
  //memcpy(&(vcopy->h), &(v->h), sizeof(Volume_header));
  vcopy->h.nsweeps = v->h.nsweeps;

  // copy the sweeps
  for (int i=0; i<v->h.nsweeps; i++) {
    // Sweep **sweeps = v->sweep;
    //Sweep **copyOfSweeps = vcopy->sweep;
    //Sweep *sweepCopy = Rsl::copy_sweep(sweeps[i]);
    //    copyOfSweeps[i] = sweepCopy;

    vcopy->sweep[i] = Rsl::copy_sweep(v->sweep[i]);
  }

  // does this do a deep copy? does it copy all the rays and data, too? Yes.
  if (debug) {  // for debugging
    cout << "copy of volume" << endl;
    print_volume(vcopy);
    cout << "end copy of volume" << endl;
  }

  return vcopy;
}



/* Internal storage conversion functions. These may be any conversion and
 * may be dynamically defined; based on the input data conversion.
 */
/*
// convert from 4 bytes <==> 2 bytes
Radx::fl32 Rsl::DZ_F(Range x) {
  return (Radx::fl32) x;
}

Radx::fl32 Rsl::VR_F(Range x) {
  return (Radx::fl32) x;
}
*/
/*
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
*/

void Rsl::print_ray_header(Ray_header header) {

  cout << "   Ray header " << endl;
  cout << "   --------" << endl;
  cout << "         nbins=" << header.nbins << endl;
  cout << "       azimuth=" << header.azimuth << endl;
  cout << "          elev=" << header.elev << endl;
  cout << "   range bin 1=" << header.range_bin1 << endl;
  cout << "     gate size=" << header.gate_size << endl;
  cout << "          bias=" << header.bias << endl;
  cout << "         scale=" << header.scale << endl;
  cout << "  altitude (m)=" << header.alt << endl;
  cout << "     allocated=";
  if (header.binDataAllocated)
    cout << "yes";
  else
    cout << "no";
  cout << endl;
  cout << "   --------" << endl;
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
      int maxToPrint = min<size_t>(20, ray->h.nbins);
      for (int i = 0; i < maxToPrint; i++) {
	cout << bin[i] << " ";
      }
      cout << endl;
    }

  }
}

void Rsl::print_sweep_header(Sweep_header header) {

  cout << "Sweep header " << endl;
  cout << "--------" << endl;
  cout << " nrays=" << header.nrays << endl;
  cout << "--------" << endl;
}



// print the Sweep and walk down the pointers to Rays and print the contents
void Rsl::print_sweep(Sweep *sweep)
{

  if (sweep == NULL)
    cout << "Sweep is NULL" << endl;
  else {
    // print the sweep header
    print_sweep_header(sweep->h);

    // print each ray
    if (sweep->ray != NULL) {
      Ray **ray = sweep->ray;
      int maxToPrint = min<size_t>(5, sweep->h.nrays);
      for (int i = 0; i < maxToPrint; i++) {
        cout << "  Ray " << i << endl;
	print_ray(ray[i]);
      }
    }
  }
}

void Rsl::setMaxBinsInSweep(Sweep *sweep, size_t nBins) {
  if (nBins > sweep->h.maxBinsInSweep)
    sweep->h.maxBinsInSweep = nBins;
}

size_t Rsl::getMaxBinsInSweep(Volume *volume, size_t sweepIndex) {
  if (volume == NULL)
    throw std::invalid_argument("getMaxBinsInSweep: volume is NULL");
  if (sweepIndex >= volume->h.nsweeps)
    throw std::invalid_argument("getMaxBinsInSweep: sweepIndex outside range");
  if (volume->sweep[sweepIndex] == NULL)
    throw std::invalid_argument("getMaxBinsInSweep: volume->sweep[sweepIndex] is NULL");
  return volume->sweep[sweepIndex]->h.maxBinsInSweep; 
}

void Rsl::findMaxNBins(Volume *volume, int *maxNBins, int *maxNRays, bool debug) {

  cout << "Finding max number of bins and rays ... " << endl;

  int maxBins = 0;
  int maxRays = 0;
  if (volume != NULL) {
    for (int i=0; i<volume->h.nsweeps; i++) {
      if (volume->sweep[i] != NULL) {
	int nrays = volume->sweep[i]->h.nrays;
	if (nrays > maxRays)
	  maxRays = nrays;
	for (int j=0; j<nrays; j++) {
	  if (volume->sweep[i]->ray[j] != NULL) {
	    int nbins = volume->sweep[i]->ray[j]->h.nbins;
	    if (nbins > maxBins)
	      maxBins = nbins;
	  } // if ray[j] != NULL
	} // for rays
      } // if sweeps[i] != NULL
    } // for sweeps
  } // end for volume != NULL
  *maxNBins = maxBins;
  *maxNRays = maxRays;

  if (debug) {
    cout << " found " << *maxNBins << " bins and " << *maxNRays << " rays" << endl;
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
      int nSweepsToPrint = volume->h.nsweeps;
      for (int i = 0; i < nSweepsToPrint; i++) {
        cout << "Sweep " << i << endl;
	print_sweep(sweep[i]);
      }
    }
  }
  cout << " ================" << endl;

}

void Rsl::verifyEqualDimensions(Volume *currDbzVol, Volume *currVelVol) {

  // check number of sweeps
  if (currDbzVol->h.nsweeps != currVelVol->h.nsweeps)
    throw "ERROR - velocity and reflectivity must has same number of sweeps";

  for (int s=0; s<currDbzVol->h.nsweeps; s++) {
    if (currDbzVol->sweep[s]->h.nrays != currVelVol->sweep[s]->h.nrays) {
      char msg[1024];
      sprintf(msg, "ERROR - velocity and reflectivity need same number of rays for each sweep\n. Check sweep %d\n", s);
      throw msg;
    }
    for (int r=0; r<currDbzVol->sweep[s]->h.nrays; r++) {
      if (currDbzVol->sweep[s]->ray[r]->h.nbins != currVelVol->sweep[s]->ray[r]->h.nbins) {
        char msg[1024];
        sprintf(msg, "ERROR - velocity and reflectivity need same number of bins for each ray\n. Check ray %d\n", r);
        throw msg;
      }
    }
  }
}

void Rsl::verifyEqualDimensionsGetMaxDimensions(Volume *currDbzVol, Volume *currVelVol, 
						float degreeThreshold, int *maxNBins, int *maxNRays,
						bool debug) {

  if ((currDbzVol == NULL) || (currVelVol == NULL))
    throw "ERROR - velocity or reflectivity Volume is NULL";

  // check number of sweeps
  if (currDbzVol->h.nsweeps != currVelVol->h.nsweeps)
    throw "ERROR - velocity and reflectivity must has same number of sweeps";

  int maxBins = 0;
  int maxRays = 0;

  for (int s=0; s<currDbzVol->h.nsweeps; s++) {
    // check for NULL
    if ((currDbzVol->sweep[s] == NULL) || (currVelVol->sweep[s] == NULL)) {
      char msg[1024];
      sprintf(msg, "ERROR - velocity or reflectivity sweep[%d] is NULL\n", s);
      throw msg;
    }
    if (currDbzVol->sweep[s]->h.nrays != currVelVol->sweep[s]->h.nrays) {
      char msg[1024];
      sprintf(msg, "ERROR - velocity and reflectivity need same number of rays for each sweep\n. Check sweep %d\n", s);
      throw msg;
    }
    // we're ok; check the number of rays
    int nrays = currDbzVol->sweep[s]->h.nrays;
    if (nrays > maxRays)
      maxRays = nrays;
    
    for (int r=0; r<currDbzVol->sweep[s]->h.nrays; r++) {
      // check for NULL
      if ((currDbzVol->sweep[s]->ray[r] == NULL) || (currVelVol->sweep[s]->ray[r] == NULL)) {
        char msg[1024];
        sprintf(msg, "ERROR - velocity or reflectivity ray is NULL; sweep[%d] ray[%d]\n.", s, r);
        throw msg;
      }
      if (currDbzVol->sweep[s]->ray[r]->h.nbins != currVelVol->sweep[s]->ray[r]->h.nbins) {
        char msg[1024];
        sprintf(msg, "ERROR - velocity and reflectivity need same number of bins for each ray\n. Check ray %d\n", r);
        throw msg;
      }
      // we're ok; check the number of bins
      int nbins = currDbzVol->sweep[s]->ray[r]->h.nbins;
      if (nbins > maxBins)
	maxBins = nbins;

      // check the elevation and azimuth; make sure they are about the same
      float diffInAzimuth = currDbzVol->sweep[s]->ray[r]->h.azimuth - currVelVol->sweep[s]->ray[r]->h.azimuth; 
      float diffInElev = currDbzVol->sweep[s]->ray[r]->h.elev - currVelVol->sweep[s]->ray[r]->h.elev; 

      if (abs(diffInAzimuth) > degreeThreshold) {
	  char msg[1024];
	  sprintf(msg, "ERROR - azimuth angle differs by more than threshold (%g): %g\n. Check sweep %d ray %d\n", 
		  degreeThreshold, diffInAzimuth, s, r);
	  throw msg;
      }

      if (abs(diffInElev) > degreeThreshold) { 
	  char msg[1024];
	  sprintf(msg, "ERROR - elevation angle differs by more than threshold (%g): %g\n. Check sweep %d ray %d\n", 
		  degreeThreshold, diffInElev, s, r);
	  throw msg;
      }
    } // for rays
  } // for sweeps

  *maxNBins = maxBins;
  *maxNRays = maxRays;
  
  if (debug) 
    cout << " found " << *maxNBins << " bins and " << *maxNRays << " rays" << endl;

}


float Rsl::getValue(Volume *volume, size_t sweepIndex, size_t rayIndex, size_t binIndex) {
  if (volume == NULL) 
    throw std::invalid_argument("getValue: volume is NULL");
  if (sweepIndex >= volume->h.nsweeps) 
    throw std::invalid_argument("getValue: sweepIndex outside range");
  if (volume->sweep[sweepIndex] == NULL) 
    throw std::invalid_argument("getValue: volume->sweep[sweepIndex] is NULL");
  if (rayIndex >= volume->sweep[sweepIndex]->h.nrays) 
    throw std::invalid_argument("getValue: rayIndex outside range");
  if (volume->sweep[sweepIndex]->ray[rayIndex] == NULL) 
    throw std::invalid_argument("getValue: volume->sweep[sweepIndex]->ray[rayIndex] is NULL");
  if (binIndex >= volume->sweep[sweepIndex]->ray[rayIndex]->h.nbins) 
    throw std::invalid_argument("getValue: binIndex outside range");
  if (volume->sweep[sweepIndex]->ray[rayIndex]->range == NULL) 
    throw std::invalid_argument("getValue: volume->sweep[sweepIndex]->ray[rayIndex]->range is NULL");
  return volume->sweep[sweepIndex]->ray[rayIndex]->range[binIndex];
}
