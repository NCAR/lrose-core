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
///////////////////////////////////////////////////////////////
// ConvStratFinder.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2014
//
////////////////////////////////////////////////////////////////////
//
// ConvStratFinder partitions stratiform and convective regions in a
// Cartesian radar volume
//
/////////////////////////////////////////////////////////////////////

#include <cmath>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
#include <radar/ConvStratFinder.hh>
using namespace std;

const fl32 ConvStratFinder::_missing = -9999.0;

// Constructor

ConvStratFinder::ConvStratFinder()

{

  _debug = false;
  _verbose = false;

  _minValidHtKm = 0.0;
  _maxValidHtKm = 30.0;
  _minValidDbz = 10.0;
  _dbzForDefiniteConvection = 53;
  _textureRadiusKm = 5.0;
  _minValidFractionForTexture = 0.33; 
  _minTextureForConvection = 15.0; 

  _minConvRadiusKm = 1.0;
  _maxConvRadiusKm = 5.0;
  _dbzForMinRadius = 22.5;
  _dbzForMaxRadius = 42.5;
  _backgroundRadiusKm = 11.0;

  _nx = _ny = 0;
  _dx = _dy = 0.0;
  _minx = _miny = 0;
  _nxy = _nxyz = 0;
  _projIsLatLon = false;

}

// destructor

ConvStratFinder::~ConvStratFinder()

{
  freeArrays();
}

//////////////////////////////////////////////////
// Compute the partition

int ConvStratFinder::computePartition(const fl32 *dbz, 
                                      fl32 dbzMissingVal)
{
  
  int iret = 0;
  PMU_auto_register("ConvStratFinder::partition()");

  // allocate the arrays

  _allocArrays();
  
  // set geometry in km
  
  if (_projIsLatLon) {
    double meanLat = (_miny + _ny * _dy / 2.0);
    double cosLat = cos(meanLat * DEG_TO_RAD);
    _dy = _dy * KM_PER_DEG_AT_EQ;
    _dx = _dx * KM_PER_DEG_AT_EQ * cosLat;
  }

  // compute min and max vert indices

  _minIz = 0;
  _maxIz = _zKm.size() - 1;
  
  for (size_t iz = 0; iz < _zKm.size(); iz++) {
    double zz = _zKm[iz];
    if (zz <= _minValidHtKm) {
      _minIz = iz;
    }
    if (zz <= _maxValidHtKm) {
      _maxIz = iz;
    }
  } // iz

  // set dbz field to missing if below the min threshold
  
  fl32 *volDbz = _volDbz.dat();
  for (int ii = 0; ii < _nxyz; ii++) {
    if (dbz[ii] == dbzMissingVal || dbz[ii] < _minValidDbz) {
      volDbz[ii] = _missing;
    } else {
      volDbz[ii] = dbz[ii];
    }
  }

  // compute the circular kernel
  
  _computeKernels();
  
  if (_verbose) {
    _printSettings(cerr);
  }
  
  // compute column maxima
  
  _computeColMax();

  // compute the background dbz
  
  _computeBackgroundDbz();

  // compute spatial texture
  
  _computeTexture();
  
  // set the convective/stratiform partition
  
  _setPartition();

  return iret;

}

/////////////////////////////////////////////////////////
// allocate the arrays

void ConvStratFinder::_allocArrays()
  
{

  _partition.alloc(_nxy);
  _convDbz.alloc(_nxyz);
  _stratDbz.alloc(_nxyz);

  _volDbz.alloc(_nxyz);
  _volTexture.alloc(_nxyz);

  _sumTexture.alloc(_nxy);
  _nTexture.alloc(_nxy);
  _meanTexture.alloc(_nxy);
  _maxTexture.alloc(_nxy);
  _fractionActive.alloc(_nxy);
  _colMaxDbz.alloc(_nxy);
  _backgroundDbz.alloc(_nxy);
  _convRadiusKm.alloc(_nxy);

}

/////////////////////////////////////////////////////////
// free the arrays

void ConvStratFinder::freeArrays()
  
{

  _partition.free();
  _convDbz.free();
  _stratDbz.free();

  _volDbz.free();
  _volTexture.free();

  _sumTexture.free();
  _nTexture.free();
  _meanTexture.free();
  _maxTexture.free();
  _fractionActive.free();
  _colMaxDbz.free();
  _backgroundDbz.free();
  _convRadiusKm.free();

}

/////////////////////////////////////////////////////////
// compute the column maximum

void ConvStratFinder::_computeColMax()
  
{

  PMU_auto_register("ConvStratFinder::_computeColMax()");

  // get data pointers

  fl32 *colMaxDbz = _colMaxDbz.dat();
  
  // initialize

  for (int ii = 0; ii < _nxy; ii++) {
    colMaxDbz[ii] = _missing;
  }

  fl32 *dbz = _volDbz.dat();
  
  for (int iz = _minIz; iz <= _maxIz; iz++) {
    for (int iy = 0; iy < _ny; iy++) {
      for (int ix = 0; ix < _nx; ix++) {
        
        int ii = ix + iy * _nx + iz * _nxy;
        int jj = ix + iy * _nx;
        fl32 dbzVal = dbz[ii];
        
        if (dbzVal == _missing) {
          continue;
        }

        if (dbzVal > colMaxDbz[jj]) {
          colMaxDbz[jj] = dbzVal;
        }

      } // ix
    } // iy
  } // iz
  
  // compute fraction covered array for texture kernel
  // we use the column maximum dbz to find points with coverage

  fl32 *fractionTexture = _fractionActive.dat();
  memset(fractionTexture, 0, _nxy * sizeof(fl32));
  for (int iy = _nyTexture; iy < _ny - _nyTexture; iy++) {
    for (int ix = _nxTexture; ix < _nx - _nxTexture; ix++) {
      int jcenter = ix + iy * _nx;
      double count = 0;
      for (size_t ii = 0; ii < _textureKernelOffsets.size(); ii++) {
        int jj = jcenter + _textureKernelOffsets[ii];
        double val = colMaxDbz[jj];
        if (val >= _minValidDbz) {
          count++;
        }
      } // ii
      double fraction = count / _textureKernelOffsets.size();
      fractionTexture[jcenter] = fraction;
    } // ix
  } // iy

}

/////////////////////////////////////////////////////////
// compute the spatial texture

void ConvStratFinder::_computeTexture()
  
{

  PMU_auto_register("ConvStratFinder::_computeTexture()");

  // array pointers

  fl32 *volTexture = _volTexture.dat();
  fl32 *sumTexture = _sumTexture.dat();
  fl32 *nTexture = _nTexture.dat();
  fl32 *meanTexture = _meanTexture.dat();
  fl32 *maxTexture = _maxTexture.dat();
  fl32 *fractionTexture = _fractionActive.dat();

  // initialize
  
  memset(nTexture, 0, _nxy * sizeof(fl32));
  memset(sumTexture, 0, _nxy * sizeof(fl32));
  
  for (int ii = 0; ii < _nxy; ii++) {
    meanTexture[ii] = _missing;
    maxTexture[ii] = -1.0e99;
  }
  for (int ii = 0; ii < _nxyz; ii++) {
    volTexture[ii] = _missing;
  }
  
  // set up threads for computing texture at each level

  const fl32 *dbz = _volDbz.dat();
  vector<ComputeTexture *> threads;
  for (int iz = _minIz; iz <= _maxIz; iz++) {
    size_t zoffset = iz * _nxy;
    ComputeTexture *thread = new ComputeTexture(iz);
    thread->setGridSize(_nx, _ny);
    thread->setKernelSize(_nxTexture, _nyTexture);
    thread->setMinValidFraction(_minValidFractionForTexture);
    thread->setDbz(dbz + zoffset, _missing);
    thread->setFractionCovered(fractionTexture);
    thread->setKernelOffsets(_textureKernelOffsets);
    thread->setTextureArray(volTexture + zoffset);
    threads.push_back(thread);
  }

  // set threads going

  for (size_t ii = 0; ii < threads.size(); ii++) {
    threads[ii]->signalRunToStart();
  }

  // wait until they are done

  for (size_t ii = 0; ii < threads.size(); ii++) {
    threads[ii]->waitForRunToComplete();
  }

  // delete threads

  for (size_t ii = 0; ii < threads.size(); ii++) {
    delete threads[ii];
  }
  threads.clear();

  // compute mean texture at each point

  int jcenter = 0;
  for (int iy = 0; iy < _ny; iy++) {
    for (int ix = 0; ix < _nx; ix++, jcenter++) {

      if (fractionTexture[jcenter] < _minValidFractionForTexture) {
        continue;
      }

      int zcenter = jcenter + _minIz * _nxy;
      
      for (int iz = _minIz; iz <= _maxIz; iz++, zcenter += _nxy) {
        fl32 texture = volTexture[zcenter];
        if (texture != _missing) {
          sumTexture[jcenter] += texture;
          nTexture[jcenter]++;
          if (texture > maxTexture[jcenter]) {
            maxTexture[jcenter] = texture;
          }
        }
      } // iz

    } // ix
  } // iy
        
  // compute mean texture and max texture
  
  int nLevelsUsed = _maxIz - _minIz + 1;
  int minLevels = (int) (nLevelsUsed * _minValidFractionForTexture + 0.5);
  
  for (int ii = 0; ii < _nxy; ii++) {
    if (nTexture[ii] >= minLevels && sumTexture[ii] > 0) {
      meanTexture[ii] = sumTexture[ii] / nTexture[ii];
    }
  }

}

/////////////////////////////////////////////////////////
// set the convective/stratiform partition

void ConvStratFinder::_setPartition()
  
{

  PMU_auto_register("ConvStratFinder::_setPartition()");

  // get data pointers
  
  const fl32 *colMaxDbz = _colMaxDbz.dat();
  const fl32 *texture = _meanTexture.dat();
  ui08 *partition = _partition.dat();
  
  // initialize

  memset(partition, 0, _nxy * sizeof(ui08));
  
  // load up convective flag at points surrounding the
  // central grid point

  for (int iy = 0; iy < _ny; iy++) {
    for (int ix = 0; ix < _nx; ix++) {
      // set convective partition
      int index = ix + iy * _nx;
      if (colMaxDbz[index] >= _dbzForDefiniteConvection ||
          texture[index] >= _minTextureForConvection) {
        _setPartitionExpanded(ix, iy, index);
      }
    } // ix
  } // iy

  // set stratiform partition
  
  for (int iy = 0; iy < _ny; iy++) {
    for (int ix = 0; ix < _nx; ix++) {
      int index = ix + iy * _nx;
      if (colMaxDbz[index] != _missing) {
        if (partition[index] == CATEGORY_MISSING) {
          partition[index] = CATEGORY_STRATIFORM;
        }
      }
    } // ix
  } // iy

  // load up the partitioned dbz arrays

  const fl32 *dbz = _volDbz.dat();
  fl32 *convDbz = _convDbz.dat();
  fl32 *stratDbz = _stratDbz.dat();

  for (int ii = 0; ii < _nxyz; ii++) {
    convDbz[ii] = _missing;
    stratDbz[ii] = _missing;
  }

  for (int iy = 0, ii = 0; iy < _ny; iy++) {
    for (int ix = 0; ix < _nx; ix++, ii++) {
      if (partition[ii] == CATEGORY_CONVECTIVE) {
        int jj = ii;
        for (int iz = 0; iz < (int) _zKm.size(); iz++, jj += _nxy) {
          convDbz[jj] = dbz[jj];
        } // iz
      } else if (partition[ii] == CATEGORY_STRATIFORM) {
        int jj = ii;
        for (int iz = 0; iz < (int) _zKm.size(); iz++, jj += _nxy) {
          stratDbz[jj] = dbz[jj];
        } // iz
      }
    } // ix
  } // iy

}

//////////////////////////////////////
// compute the kernels for this grid

void ConvStratFinder::_computeKernels()

{

  PMU_auto_register("ConvStratFinder::_computeKernels()");
    
  // texture kernel

  _textureKernelOffsets.clear();

  _nyTexture = (int) floor(_textureRadiusKm / _dy + 0.5);
  _nxTexture = (int) floor(_textureRadiusKm / _dx + 0.5);
  
  if (_verbose) {
    cerr << "Texture kernel size:" << endl;
    cerr << "  ny: " << _nyTexture << endl;
    cerr << "  nx: " << _nxTexture << endl;
  }
  
  for (int jdy = -_nyTexture; jdy <= _nyTexture; jdy++) {
    double yy = jdy * _dy;
    for (int jdx = -_nxTexture; jdx <= _nxTexture; jdx++) {
      double xx = jdx * _dx;
      double radius = sqrt(yy * yy + xx * xx);
      if (radius <= _textureRadiusKm) {
        ssize_t offset = jdx + jdy * _nx;
        _textureKernelOffsets.push_back(offset);
      }
    }
  }

  // background kernel

  _backgroundKernelOffsets.clear();
  
  _nyBackground = (int) floor(_backgroundRadiusKm / _dy + 0.5);
  _nxBackground = (int) floor(_backgroundRadiusKm / _dx + 0.5);
  
  if (_verbose) {
    cerr << "Background kernel size:" << endl;
    cerr << "  ny: " << _nyBackground << endl;
    cerr << "  nx: " << _nxBackground << endl;
  }
  
  for (int jdy = -_nyBackground; jdy <= _nyBackground; jdy++) {
    double yy = jdy * _dy;
    for (int jdx = -_nxBackground; jdx <= _nxBackground; jdx++) {
      double xx = jdx * _dx;
      double radius = sqrt(yy * yy + xx * xx);
      if (radius <= _backgroundRadiusKm) {
        ssize_t offset = jdx + jdy * _nx;
        _backgroundKernelOffsets.push_back(offset);
      }
    }
  }

}

////////////////////////////////////////////////////////////
// set partition, expanding convection by computed radius

void ConvStratFinder::_setPartitionExpanded(int ix, int iy, int index)
  
{

  // get the convetive radius for this point
  
  double backgroundDbz = _backgroundDbz.dat()[index];
  double radius = _computeConvRadiusKm(backgroundDbz);
  _convRadiusKm.dat()[index] = radius;
  double radSq = radius * radius;
  int ny = (int) floor(radius / _dy + 0.5);
  int nx = (int) floor(radius / _dx + 0.5);

  ui08 *partition = _partition.dat();

  for (int jy = -ny; jy <= ny; jy++) {
    double yy = jy * _dy;
    for (int jx = -nx; jx <= nx; jx++) {
      double xx = jx * _dx;
      double rSq = yy * yy + xx * xx;
      if (rSq <= radSq) {
        int ky = iy + jy;
        int kx = ix + jx;
        if (ky >= 0 && ky < _ny - 1 &&
            kx >= 0 && kx < _nx - 1) {
          int kk = kx + ky * _nx;
          partition[kk] = CATEGORY_CONVECTIVE;
        } // if (ky >= 0 ...
      } // if (rSq <= radSq)
    } // jx
  } // jy

}

//////////////////////////////////////
// print parameter settings

void ConvStratFinder::_printSettings(ostream &out)

{

  out << "========== ConvStratFinder settings ============" << endl;

  out << "  _minValidHtKm: " << _minValidHtKm << endl;
  out << "  _maxValidHtKm: " << _maxValidHtKm << endl;
  out << "  _minValidDbz: " << _minValidDbz << endl;
  out << "  _dbzForDefiniteConvection: " << _dbzForDefiniteConvection << endl;
  out << "  _textureRadiusKm: " << _textureRadiusKm << endl;
  out << "  _minValidFractionForTexture: " << _minValidFractionForTexture << endl;
  out << "  _minTextureForConvection: " << _minTextureForConvection << endl;

  out << "  _nx: " << _nx << endl;
  out << "  _ny: " << _ny << endl;
  out << "  _minx: " << _minx << endl;
  out << "  _miny: " << _miny << endl;
  out << "  _dx: " << _dx << endl;
  out << "  _dy: " << _dy << endl;

  out << "  nz: " << _zKm.size();
  for (size_t ii = 0; ii < _zKm.size(); ii++) {
    out << "    ii, z: " << ii << ", " << _zKm[ii] << endl;
  }

  out << "  _projIsLatLon: " << (_projIsLatLon?"Y":"N:") << endl;

  out << "  _nxy: " << _nxy << endl;
  out << "  _nxyz: " << _nxyz << endl;
  out << "  _minIz: " << _minIz << endl;
  out << "  _maxIz: " << _maxIz << endl;

  out << "  _nxTexture: " << _nxTexture << endl;
  out << "  _nyTexture: " << _nyTexture << endl;
  out << "  _nxBackground: " << _nxBackground << endl;
  out << "  _nyBackground: " << _nyBackground << endl;

}

////////////////////////////////////////////////////////////////////
// Compute the radius of convective influence from the background
// reflectivity.
// Given definite convection at a point (see above),
// we set all points within the computed radius to be convective.

double ConvStratFinder::_computeConvRadiusKm(double backgroundDbz) 
{
  if (backgroundDbz == _missing) {
    return _minConvRadiusKm;
  }
  if (backgroundDbz < _dbzForMinRadius) {
    return _minConvRadiusKm;
  }
  if (backgroundDbz > _dbzForMaxRadius) {
    return _maxConvRadiusKm;
  }
  double radius = _minConvRadiusKm +
    (backgroundDbz - _dbzForMinRadius) * _radiusSlope;
  return radius;
}

/////////////////////////////////////////////
// Compute the background dbz
// at each point in the col max dbz

void ConvStratFinder::_computeBackgroundDbz()
{

  // get data pointers

  fl32 *colMaxDbz = _colMaxDbz.dat();
  fl32 *backgroundDbz = _backgroundDbz.dat();
  
  // initialize

  for (int ii = 0; ii < _nxy; ii++) {
    backgroundDbz[ii] = _missing;
  }

  // background is mean dbz in a circle around a point
  
  for (int iy = _nyBackground; iy < _ny - _nyBackground; iy++) {
    
    int icenter = _nxBackground + iy * _nx;
    
    for (int ix = _nxBackground; ix < _nx - _nxBackground; ix++, icenter++) {
      
      double nn = 0.0;
      double sum = 0.0;
      
      for (size_t ii = 0; ii < _backgroundKernelOffsets.size(); ii++) {
        int kk = icenter + _backgroundKernelOffsets[ii];
        double dbz = colMaxDbz[kk];
        if (dbz != _missing) {
          sum += dbz;
        }
        nn++;
      } // ii
      
      if (nn >= 0) {
        double mean = sum / nn;
        backgroundDbz[icenter] = mean;
      } else {
        backgroundDbz[icenter] = 0.0;
      }
      
    } // ix
    
  } // iy

}

///////////////////////////////////////////////////////////////
// ComputeTexture inner class
//
// Compute texture for 1 level in a thread
//
///////////////////////////////////////////////////////////////

// Constructor

ConvStratFinder::ComputeTexture::ComputeTexture(int iz) :
        TaThread(),
        _iz(iz)
{
  char name[128];
  sprintf(name, "ComputeTexture-level-%d", _iz);
  setThreadName(name);
  _dbz = NULL;
  _fractionCovered = NULL;
  _texture = NULL;
  _nx = _ny = 0;
  _nxTexture = _nyTexture = 0;
}  


ConvStratFinder::ComputeTexture::~ComputeTexture() 
{
}

// override run method
// compute texture at each point in plane

void ConvStratFinder::ComputeTexture::run()
{

  // check for validity
  
  if (_dbz == NULL ||
      _fractionCovered == NULL ||
      _texture == NULL) {
    cerr << "ERROR - ComputeTexture::run" << endl;
    cerr << "  Initialization not complete" << endl;
    return;
  }
  
  // initialize texture to missing

  for (int ii = 0; ii < _nx * _ny; ii++) {
    _texture[ii] = _missingVal;
  }

  // compute texture at each point in the plane

  int minPtsForTexture = 
    (int) (_minValidFraction * _kernelOffsets.size() + 0.5);
  
  for (int iy = _nyTexture; iy < _ny - _nyTexture; iy++) {
    
    int icenter = _nxTexture + iy * _nx;
    
    for (int ix = _nxTexture; ix < _nx - _nxTexture; ix++, icenter++) {
      
      if (_fractionCovered[icenter] < _minValidFraction) {
        continue;
      }
      
      // compute texture in circular kernel around point
      // first we compute the standard deviation of the square of dbz
      // then we take the square root of the sdev
      
      double nn = 0.0;
      double sum = 0.0;
      double sumSq = 0.0;
      
      for (size_t ii = 0; ii < _kernelOffsets.size(); ii++) {
        int kk = icenter + _kernelOffsets[ii];
        double val = _dbz[kk];
        if (val != _missingVal) {
          double dbzSq = val * val;
          sum += dbzSq;
          sumSq += dbzSq * dbzSq;
          nn++;
        }
      } // ii
      if (nn >= minPtsForTexture) {
        double mean = sum / nn;
        double var = sumSq / nn - (mean * mean);
        if (var < 0.0) {
          var = 0.0;
        }
        double sdev = sqrt(var);
        _texture[icenter] = sqrt(sdev);
      }
      
    } // ix
    
  } // iy
  
}

