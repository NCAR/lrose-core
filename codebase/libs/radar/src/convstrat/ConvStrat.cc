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
// ConvStrat.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2014
//
////////////////////////////////////////////////////////////////////
//
// ConvStrat partitions stratiform and convective regions in a
// Cartesian radar volume
//
/////////////////////////////////////////////////////////////////////

#include <cmath>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
#include <radar/ConvStrat.hh>
using namespace std;

const fl32 ConvStrat::_missing = -9999.0;

// Constructor

ConvStrat::ConvStrat()

{

  _debug = false;
  _verbose = false;

  _minValidHtKm = 0.0;
  _maxValidHtKm = 30.0;
  _minValidDbz = 10.0;
  _dbzForDefiniteConvection = 53;
  _convectiveRadiusKm = 5.0;
  _textureRadiusKm = 5.0;
  _minValidFractionForTexture = 0.33; 
  _minTextureForConvection = 15.0; 

  _nx = _ny = 0;
  _dx = _dy = 0.0;
  _minx = _miny = 0;
  _nxy = _nxyz = 0;
  _projIsLatLon = false;

}

// destructor

ConvStrat::~ConvStrat()

{
  freeArrays();
}

//////////////////////////////////////////////////
// Compute the partition

int ConvStrat::computePartition(const fl32 *dbz, 
                                fl32 dbzMissingVal)
{
  
  int iret = 0;
  PMU_auto_register("ConvStrat::partition()");

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
  
  fl32 *volDbz = _volDbz.buf();
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
  
  // compute spatial texture
  
  _computeTexture();
  
  // set the convective/stratiform partition
  
  _setPartition();

  return iret;

}

/////////////////////////////////////////////////////////
// allocate the arrays

void ConvStrat::_allocArrays()
  
{

  _partition.alloc(_nxy);
  _convDbz.alloc(_nxyz);
  _stratDbz.alloc(_nxyz);

  _volDbz.alloc(_nxyz);
  _volTexture.alloc(_nxyz);

  _sumTexture.alloc(_nxy);
  _nTexture.alloc(_nxy);
  _meanTexture.alloc(_nxy);
  _fractionActive.alloc(_nxy);
  _colMaxDbz.alloc(_nxy);
  _convFromColMax.alloc(_nxy);
  _convFromTexture.alloc(_nxy);

}

/////////////////////////////////////////////////////////
// free the arrays

void ConvStrat::freeArrays()
  
{

  _partition.free();
  _convDbz.free();
  _stratDbz.free();

  _volDbz.free();
  _volTexture.free();

  _sumTexture.free();
  _nTexture.free();
  _meanTexture.free();
  _fractionActive.free();
  _colMaxDbz.free();
  _convFromColMax.free();
  _convFromTexture.free();

}

/////////////////////////////////////////////////////////
// compute the column maximum

void ConvStrat::_computeColMax()
  
{

  PMU_auto_register("ConvStrat::_computeColMax()");

  // get date pointers

  fl32 *colMaxDbz = _colMaxDbz.buf();
  
  // initialize

  for (int ii = 0; ii < _nxy; ii++) {
    colMaxDbz[ii] = _missing;
  }

  fl32 *dbz = _volDbz.buf();
  
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

  fl32 *fractionTexture = _fractionActive.buf();
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

void ConvStrat::_computeTexture()
  
{

  PMU_auto_register("ConvStrat::_computeTexture()");

  // array pointers

  fl32 *volTexture = _volTexture.buf();
  fl32 *sumTexture = _sumTexture.buf();
  fl32 *nTexture = _nTexture.buf();
  fl32 *meanTexture = _meanTexture.buf();
  fl32 *fractionTexture = _fractionActive.buf();

  // initialize
  
  memset(nTexture, 0, _nxy * sizeof(fl32));
  memset(sumTexture, 0, _nxy * sizeof(fl32));
  
  for (int ii = 0; ii < _nxy; ii++) {
    meanTexture[ii] = _missing;
  }
  for (int ii = 0; ii < _nxyz; ii++) {
    volTexture[ii] = _missing;
  }
  
  // set up threads for computing texture at each level

  const fl32 *dbz = _volDbz.buf();
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
        }
      } // iz

    } // ix
  } // iy
        
  // compute mean texture
  
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

void ConvStrat::_setPartition()
  
{

  PMU_auto_register("ConvStrat::_setPartition()");

  // get date pointers
  
  const fl32 *colMaxDbz = _colMaxDbz.buf();
  const fl32 *texture = _meanTexture.buf();
  ui08 *convFromColMax = _convFromColMax.buf();
  ui08 *convFromTexture = _convFromTexture.buf();
  ui08 *partition = _partition.buf();
  
  // initialize

  memset(convFromColMax, 0, _nxy * sizeof(ui08));
  memset(convFromTexture, 0, _nxy * sizeof(ui08));
  memset(partition, 0, _nxy * sizeof(ui08));
  
  // load up convective flag at points surrounding the
  // central grid point
        
  for (int iy = _nyConv; iy < _ny - _nyConv; iy++) {
    for (int ix = _nxConv; ix < _nx - _nxConv; ix++) {
      
      int ii = ix + iy * _nx;

      // set convective flag

      if (colMaxDbz[ii] >= _dbzForDefiniteConvection) {
        for (size_t kk = 0; kk < _convKernelOffsets.size(); kk++) {
          int jj = ii + _convKernelOffsets[kk];
          convFromColMax[jj] = CATEGORY_CONVECTIVE;
          partition[jj] = CATEGORY_CONVECTIVE;
        }
      }

      if (texture[ii] >= _minTextureForConvection) {
        for (size_t kk = 0; kk < _convKernelOffsets.size(); kk++) {
          int jj = ii + _convKernelOffsets[kk];
          convFromTexture[jj] = CATEGORY_CONVECTIVE;
          partition[jj] = CATEGORY_CONVECTIVE;
        }
      }

      // set stratiform flag

      if (colMaxDbz[ii] != _missing) {
        if (partition[ii] == CATEGORY_MISSING) {
          partition[ii] = CATEGORY_STRATIFORM;
        }
      }

    } // ix
  } // iy

  // load up the partitioned dbz arrays

  const fl32 *dbz = _volDbz.buf();
  fl32 *convDbz = _convDbz.buf();
  fl32 *stratDbz = _stratDbz.buf();

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

void ConvStrat::_computeKernels()

{

  PMU_auto_register("ConvStrat::_computeKernels()");
    
  // texture kernel

  _textureKernelOffsets.clear();

  _nyTexture = (int) floor(_textureRadiusKm / _dy + 0.5);
  _nxTexture = (int) floor(_textureRadiusKm / _dx + 0.5);
  
  if (_verbose) {
    cerr << "Texture kernel size:" << endl;
    cerr << "  idy: " << _nyTexture << endl;
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

  // convective kernel

  _convKernelOffsets.clear();

  _nyConv = (int) floor(_convectiveRadiusKm / _dy + 0.5);
  _nxConv = (int) floor(_convectiveRadiusKm / _dx + 0.5);

  if (_verbose) {
    cerr << "Convective kernel size:" << endl;
    cerr << "  idy: " << _nyConv << endl;
    cerr << "  nx: " << _nxConv << endl;
  }
  
  for (int jdy = -_nyConv; jdy <= _nyConv; jdy++) {
    double yy = jdy * _dy;
    for (int jdx = -_nxConv; jdx <= _nxConv; jdx++) {
      double xx = jdx * _dx;
      double radius = sqrt(yy * yy + xx * xx);
      if (radius <= _convectiveRadiusKm) {
        ssize_t offset = jdx + jdy * _nx;
        _convKernelOffsets.push_back(offset);
      }
    }
  }

}

//////////////////////////////////////
// print parameter settings

void ConvStrat::_printSettings(ostream &out)

{

  out << "========== ConvStrat settings ============" << endl;

  out << "  _minValidHtKm: " << _minValidHtKm << endl;
  out << "  _maxValidHtKm: " << _maxValidHtKm << endl;
  out << "  _minValidDbz: " << _minValidDbz << endl;
  out << "  _dbzForDefiniteConvection: " << _dbzForDefiniteConvection << endl;
  out << "  _convectiveRadiusKm: " << _convectiveRadiusKm << endl;
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
  out << "  _nxConv: " << _nxConv << endl;
  out << "  _nyConv: " << _nyConv << endl;



}

///////////////////////////////////////////////////////////////
// ComputeTexture inner class
//
// Compute texture for 1 level in a thread
//
///////////////////////////////////////////////////////////////

// Constructor

ConvStrat::ComputeTexture::ComputeTexture(int iz) :
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


ConvStrat::ComputeTexture::~ComputeTexture() 
{
}

// override run method
// compute texture at each point in plane

void ConvStrat::ComputeTexture::run()
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

