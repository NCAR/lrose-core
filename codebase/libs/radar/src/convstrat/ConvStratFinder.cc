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

#include <cassert>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
#include <radar/ConvStratFinder.hh>
#include <rapmath/PlaneFit.hh>
using namespace std;

const fl32 ConvStratFinder::_missingFl32 = -9999.0;
const ui08 ConvStratFinder::_missingUi08 = ConvStratFinder::CATEGORY_MISSING;

// Constructor

ConvStratFinder::ConvStratFinder()

{

  _debug = false;
  _verbose = false;

  _minValidHtKm = 0.0;
  _maxValidHtKm = 30.0;
  _minValidDbz = 0.0;
  _dbzForDefiniteConvection = 53;
  _textureRadiusKm = 5.0;
  _minValidFractionForTexture = 0.33; 
  _minTextureForConvection = 15.0; 
  _maxTextureForStratiform = 11.0; 

  _minConvRadiusKm = 1.0;
  _maxConvRadiusKm = 5.0;
  _dbzForMinRadius = 22.5;
  _dbzForMaxRadius = 42.5;
  _backgroundRadiusKm = 11.0;
  _dbzForEchoTops = 18.0;

  _nx = _ny = 0;
  _dx = _dy = 0.0;
  _minx = _miny = 0;
  _nxy = _nxyz = 0;
  _projIsLatLon = false;
  _gridSet = false;

  _specifyLevelsByHtValues = true;
  _shallowHtKm = 4.0;
  _deepHtKm = 8.0;

  _textureLimitLow = 0.0;
  _textureLimitHigh = 30.0;

  _convInterestThreshold = 0.5;

}

// destructor

ConvStratFinder::~ConvStratFinder()

{
  freeArrays();
}

////////////////////////////////////////////////////////////////////
// Set grid details

void ConvStratFinder::setGrid(size_t nx, size_t ny, 
                              double dx, double dy,
                              double minx, double miny,
                              const vector<double> &zKm,
                              bool projIsLatLon /* = false */)
{

  _nx = nx;
  _ny = ny;
  _dx = dx;
  _dy = dy;
  _minx = minx;
  _miny = miny;
  _zKm = zKm;
  _nxy = _nx * _ny;
  _nxyz = _nxy * _zKm.size();
  _projIsLatLon = projIsLatLon;
  _specifyLevelsByHtValues = true;
  _gridSet = true;

  // allocate the arrays and set to missing

  _allocArrays();
  _initToMissing();
  
}

////////////////////////////////////////////////////////////////////
// Set the freezing level and divergence level as km MSL values

void ConvStratFinder::setConstantHtThresholds(double shallowHtKm,
                                              double deepHtKm)

{
  _specifyLevelsByHtValues = true;
  _shallowHtKm = shallowHtKm;
  _deepHtKm = deepHtKm;
}

////////////////////////////////////////////////////////////////////
// Set the freezing level and divergence level as grids
// These must be on the same grid as the radar DBZ data

void ConvStratFinder::setGridHtThresholds(const fl32 *shallowHtGrid,
                                          const fl32 *deepHtGrid,
                                          size_t nptsPlane)
{
  assert(_gridSet);
  assert(nptsPlane == _nxy);
  memcpy(_shallowHtGrid.dat(), shallowHtGrid, _nxy * sizeof(fl32));
  memcpy(_deepHtGrid.dat(), deepHtGrid, _nxy * sizeof(fl32));
}

//////////////////////////////////////////////////
// Compute the partition

int ConvStratFinder::computePartition(const fl32 *dbz, 
                                      fl32 dbzMissingVal)
{
  
  assert(_gridSet);

  int iret = 0;
  PMU_auto_register("ConvStratFinder::partition()");

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
  
  fl32 *volDbz = _dbz3D.dat();
  for (size_t ii = 0; ii < _nxyz; ii++) {
    if (dbz[ii] == dbzMissingVal || dbz[ii] < _minValidDbz) {
      volDbz[ii] = _missingFl32;
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

  // compute convectivity interest
  
  _computeInterest();

  // perform clumping on the interest field

  _performClumping();

  // set the 3D version of the partition

  _setPartition3D();

  // compute the properties from the vertical profiles of texture

  _computeProps();
  
  // finalize the convective/stratiform partitions,
  // expanding as required
  
  _finalizePartition();

  return iret;

}

/////////////////////////////////////////////////////////
// allocate the arrays

void ConvStratFinder::_allocArrays()
  
{

  _dbz3D.alloc(_nxyz);
  _shallowHtGrid.alloc(_nxy);
  _deepHtGrid.alloc(_nxy);

  _partition3D.alloc(_nxyz);
  _partition.alloc(_nxy);
  _partitionLow.alloc(_nxy);
  _partitionMid.alloc(_nxy);
  _partitionHigh.alloc(_nxy);
  
  _convDbz.alloc(_nxyz);
  _stratDbz.alloc(_nxyz);

  _texture3D.alloc(_nxyz);
  _interest3D.alloc(_nxyz);

  _meanTexture.alloc(_nxy);
  _meanTextureLow.alloc(_nxy);
  _meanTextureMid.alloc(_nxy);
  _meanTextureHigh.alloc(_nxy);
  _maxTexture.alloc(_nxy);
  
  _fractionActive.alloc(_nxy);
  _colMaxDbz.alloc(_nxy);
  _backgroundDbz.alloc(_nxy);
  _convRadiusKm.alloc(_nxy);

  _convBaseKm.alloc(_nxy);
  _convTopKm.alloc(_nxy);
  _stratBaseKm.alloc(_nxy);
  _stratTopKm.alloc(_nxy);
  _echoTopKm.alloc(_nxy);
  
}

/////////////////////////////////////////////////////////
// init arrays to missing

void ConvStratFinder::_initToMissing()
  
{

  _initToMissing(_dbz3D, _missingFl32);
  _initToMissing(_shallowHtGrid, _missingFl32);
  _initToMissing(_deepHtGrid, _missingFl32);

  _initToMissing(_partition3D, _missingUi08);
  _initToMissing(_partition, _missingUi08);
  _initToMissing(_partitionLow, _missingUi08);
  _initToMissing(_partitionMid, _missingUi08);
  _initToMissing(_partitionHigh, _missingUi08);
  
  _initToMissing(_convDbz, _missingFl32);
  _initToMissing(_stratDbz, _missingFl32);

  _initToMissing(_texture3D, _missingFl32);
  _initToMissing(_interest3D, _missingFl32);

  _initToMissing(_meanTexture, _missingFl32);
  _initToMissing(_meanTextureLow, _missingFl32);
  _initToMissing(_meanTextureMid, _missingFl32);
  _initToMissing(_meanTextureHigh, _missingFl32);
  _initToMissing(_maxTexture, _missingFl32);
  
  _initToMissing(_fractionActive, _missingFl32);
  _initToMissing(_colMaxDbz, _missingFl32);
  _initToMissing(_backgroundDbz, _missingFl32);
  _initToMissing(_convRadiusKm, _missingFl32);

  _initToMissing(_convBaseKm, _missingFl32);
  _initToMissing(_convTopKm, _missingFl32);
  _initToMissing(_stratBaseKm, _missingFl32);
  _initToMissing(_stratTopKm, _missingFl32);
  _initToMissing(_echoTopKm, _missingFl32);
  
}

void ConvStratFinder::_initToMissing(TaArray<fl32> &array,
                                     fl32 missingVal)
{
  fl32 *data = array.dat();
  for (size_t ii = 0; ii < array.size(); ii++) {
    data[ii] = missingVal;
  }
}

void ConvStratFinder::_initToMissing(TaArray<ui08> &array,
                                     ui08 missingVal)
{
  ui08 *data = array.dat();
  for (size_t ii = 0; ii < array.size(); ii++) {
    data[ii] = missingVal;
  }
}

/////////////////////////////////////////////////////////
// free the arrays

void ConvStratFinder::freeArrays()
  
{

  _dbz3D.free();
  _shallowHtGrid.free();
  _deepHtGrid.free();

  _partition3D.free();
  _partition.free();
  _partitionLow.free();
  _partitionMid.free();
  _partitionHigh.free();
  
  _convDbz.free();
  _stratDbz.free();

  _texture3D.free();
  _interest3D.free();

  _meanTexture.free();
  _meanTextureLow.free();
  _meanTextureMid.free();
  _meanTextureHigh.free();
  _maxTexture.free();
  
  _fractionActive.free();
  _colMaxDbz.free();
  _backgroundDbz.free();
  _convRadiusKm.free();

  _convBaseKm.free();
  _convTopKm.free();
  _stratBaseKm.free();
  _stratTopKm.free();
  _echoTopKm.free();
  
}

/////////////////////////////////////////////////////////
// compute the column maximum

void ConvStratFinder::_computeColMax()
  
{

  PMU_auto_register("ConvStratFinder::_computeColMax()");

  // get data pointers

  fl32 *colMaxDbz = _colMaxDbz.dat();
  
  // initialize

  for (size_t ii = 0; ii < _nxy; ii++) {
    colMaxDbz[ii] = _missingFl32;
  }

  fl32 *dbz = _dbz3D.dat();
  fl32 *topKm = _echoTopKm.dat();
  
  for (size_t iz = _minIz; iz <= _maxIz; iz++) {
    double htKm = _zKm[iz];
    size_t jj = 0;
    size_t ii = iz * _nxy;
    for (size_t iy = 0; iy < _ny; iy++) {
      for (size_t ix = 0; ix < _nx; ix++, jj++, ii++) {
        
        // size_t jj = ix + iy * _nx;
        // size_t ii = jj + iz * _nxy;
        fl32 dbzVal = dbz[ii];
        
        if (dbzVal == _missingFl32) {
          continue;
        }

        if (dbzVal > colMaxDbz[jj]) {
          colMaxDbz[jj] = dbzVal;
        }
        
        if (dbzVal >= _dbzForEchoTops) {
          topKm[jj] = htKm;
        }

      } // ix
    } // iy
  } // iz
  
  // compute fraction covered array for texture kernel
  // we use the column maximum dbz to find points with coverage

  fl32 *fractionTexture = _fractionActive.dat();
  memset(fractionTexture, 0, _nxy * sizeof(fl32));
  for (int iy = _nyTexture; iy < (int) _ny - _nyTexture; iy++) {
    for (int ix = _nxTexture; ix < (int) _nx - _nxTexture; ix++) {
      size_t jcenter = ix + iy * _nx;
      double count = 0;
      for (size_t ii = 0; ii < _textureKernelOffsets.size(); ii++) {
        size_t jj = jcenter + _textureKernelOffsets[ii].offset;
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

  fl32 *volTexture = _texture3D.dat();
  fl32 *fractionTexture = _fractionActive.dat();

  // initialize
  
  for (size_t ii = 0; ii < _nxyz; ii++) {
    volTexture[ii] = _missingFl32;
  }
  
  // set up threads for computing texture at each level

  const fl32 *dbz = _dbz3D.dat();
  const fl32 *colMaxDbz = _colMaxDbz.dat();
  vector<ComputeTexture *> threads;
  for (size_t iz = _minIz; iz <= _maxIz; iz++) {
    size_t zoffset = iz * _nxy;
    ComputeTexture *thread = new ComputeTexture(iz);
    thread->setGridSize(_nx, _ny);
    thread->setKernelSize(_nxTexture, _nyTexture);
    thread->setMinValidFraction(_minValidFractionForTexture);
    thread->setDbz(dbz + zoffset, colMaxDbz, _missingFl32);
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

}

/////////////////////////////////////////////////////////
// compute the convectivity interest

void ConvStratFinder::_computeInterest()
  
{

  // array pointers

  fl32 *texture3D = _texture3D.dat();
  fl32 *interest3D = _interest3D.dat();
  fl32 *active2D = _fractionActive.dat();
  
  // loop through the vol
  
  size_t index3D = 0;
  double textureRange = _textureLimitHigh - _textureLimitLow;
  double interestSlope = 1.0 / textureRange;
  
  for (size_t iz = 0; iz < _zKm.size(); iz++) {
    
    // loop through a plane
    
    size_t index2D = 0;
    
    for (size_t iy = 0; iy < _ny; iy++) {
      for (size_t ix = 0; ix < _nx; ix++, index2D++, index3D++) {
        
        fl32 interest = _missingFl32;
        if (active2D[index2D] >= _minValidFractionForTexture) {
          double texture = texture3D[index3D];
          if (texture < _textureLimitLow) {
            interest = 0.0;
          } else if (texture > _textureLimitHigh) {
            interest = 1.0;
          } else {
            interest = (texture - _textureLimitLow) * interestSlope;
          }
        }
        interest3D[index3D] = interest;

      } // ix
    } // iy
  } // iz
  
}

/////////////////////////////////////////////////////////
// perform clumping on the interest field

void ConvStratFinder::_performClumping()
  
{

  int nOverlap = 1;
  _nClumps = _clumping.performClumping(_nx, _ny, _zKm.size(),
                                       _interest3D.dat(),
                                       nOverlap,
                                       _convInterestThreshold);
  
  if (_verbose) {
    cerr << "ConvStratFinder::_performClumping()" << endl;
    cerr << "  N clumps: " << _nClumps << endl;
  }
  
}

/////////////////////////////////////////////////////////
// set 3d partition array

void ConvStratFinder::_setPartition3D()
  
{

  // array pointers

  ui08 *partition3D = _partition3D.dat();
  fl32 *texture3D = _texture3D.dat();
  fl32 *active2D = _fractionActive.dat();
  
  // loop through the vol
  
  size_t index3D = 0;

  for (size_t iz = 0; iz < _zKm.size(); iz++) {

    // loop through a plane
    
    size_t index2D = 0;
    
    for (size_t iy = 0; iy < _ny; iy++) {
      for (size_t ix = 0; ix < _nx; ix++, index2D++, index3D++) {
        
        if (active2D[index2D] < _minValidFractionForTexture) {
          partition3D[index3D] = CATEGORY_MISSING;
          continue;
        }
        double texture = texture3D[index3D];
        
        if (texture == _missingFl32) {
          partition3D[index3D] = CATEGORY_MISSING;
        } else if (texture >= _minTextureForConvection) {
          partition3D[index3D] = CATEGORY_CONVECTIVE;
        } else if (texture <= _maxTextureForStratiform) {
          partition3D[index3D] = CATEGORY_STRATIFORM;
        } else {
          partition3D[index3D] = CATEGORY_MIXED;
        }

      } // ix
    } // iy
  } // iz
  
}

/////////////////////////////////////////////////////////
// set properties from the vertical profile of texture

void ConvStratFinder::_computeProps()
  
{

  PMU_auto_register("ConvStratFinder::_setPropsFromTextureProfile()");

  // array pointers

  fl32 *volTexture = _texture3D.dat();
  fl32 *fractionTexture = _fractionActive.dat();

  // loop through the x/y arrays
  
  size_t jcenter = 0;
  for (size_t iy = 0; iy < _ny; iy++) {
    for (size_t ix = 0; ix < _nx; ix++, jcenter++) {
      
      if (fractionTexture[jcenter] < _minValidFractionForTexture) {
        continue;
      }
      size_t zcenter = jcenter + _minIz * _nxy;
      
      // create a vert profile of texture
      
      vector<fl32> vtext;
      for (size_t iz = _minIz; iz <= _maxIz; iz++, zcenter += _nxy) {
        fl32 texture = volTexture[zcenter];
        vtext.push_back(texture);
      } // iz

      // compute the props for this point

      _computeProps(jcenter, vtext);

    } // ix
  } // iy
        
}

/////////////////////////////////////////////////////////////
// compute the props for a vert profile at a specified point
// The index gives the location of the point in (x, y)

void ConvStratFinder::_computeProps(size_t index,
                                    vector<fl32> &textureProfile)
  
{

  double shallowHt = _shallowHtKm;
  double deepHt = _deepHtKm;
  
  if (!_specifyLevelsByHtValues) {
    shallowHt = _shallowHtGrid.dat()[index];
    deepHt = _deepHtGrid.dat()[index];
  }
  
  // compute mean texture for full column, plus the
  // low, mid and high parts of the column
  // compute max textful for full column
  // set partition values

  double nFull = 0.0, nLow = 0.0, nMid = 0.0, nHigh = 0.0;
  double sumFull = 0.0, sumLow = 0.0, sumMid = 0.0, sumHigh = 0.0;
  fl32 maxFull = _missingFl32;
  
  for (size_t ii = 0; ii < textureProfile.size(); ii++) {
    double ht = _zKm[ii];
    fl32 texture = textureProfile[ii];
    if (texture == _missingFl32) {
      continue;
    }
    if (texture > maxFull) {
      maxFull = texture;
    }
    nFull++;
    sumFull += texture;
    if (ht <= shallowHt) {
      nLow++;
      sumLow += texture;
    } else if (ht <= deepHt) {
      nMid++;
      sumMid += texture;
    } else {
      nHigh++;
      sumHigh += texture;
    }
  } // ii

  bool convectiveAny = false;
  bool stratiformAny = false;
  bool mixedAny = false;

  if (nFull > 0) {
    double mean = sumFull / nFull;
    _meanTexture.dat()[index] = mean;
  }

  if (nLow > 0) {
    double meanLow = sumLow / nLow;
    _meanTextureLow.dat()[index] = meanLow;
    if (meanLow >= _minTextureForConvection) {
      _partitionLow.dat()[index] = CATEGORY_CONVECTIVE;
      convectiveAny = true;
    } else if (meanLow <= _maxTextureForStratiform) {
      _partitionLow.dat()[index] = CATEGORY_STRATIFORM;
      stratiformAny = true;
    } else {
      _partitionLow.dat()[index] = CATEGORY_MIXED;
      mixedAny = true;
    }
  }

  if (nMid > 0) {
    double meanMid = sumMid / nMid;
    _meanTextureMid.dat()[index] = meanMid;
    if (meanMid >= _minTextureForConvection) {
      _partitionMid.dat()[index] = CATEGORY_CONVECTIVE;
      convectiveAny = true;
    } else if (meanMid <= _maxTextureForStratiform) {
      _partitionMid.dat()[index] = CATEGORY_STRATIFORM;
      stratiformAny = true;
    } else {
      _partitionMid.dat()[index] = CATEGORY_MIXED;
      mixedAny = true;
    }
  }

  if (nHigh > 0) {
    double meanHigh = sumHigh / nHigh;
    _meanTextureHigh.dat()[index] = meanHigh;
    if (meanHigh >= _minTextureForConvection) {
      _partitionHigh.dat()[index] = CATEGORY_CONVECTIVE;
      convectiveAny = true;
    } else if (meanHigh <= _maxTextureForStratiform) {
      _partitionHigh.dat()[index] = CATEGORY_STRATIFORM;
      stratiformAny = true;
    } else {
      _partitionHigh.dat()[index] = CATEGORY_MIXED;
      mixedAny = true;
    }
  }

  if (convectiveAny) {
    _partition.dat()[index] = CATEGORY_CONVECTIVE;
  } else if (stratiformAny) {
    _partition.dat()[index] = CATEGORY_STRATIFORM;
  } else if (mixedAny) {
    _partition.dat()[index] = CATEGORY_MIXED;
  }

  _maxTexture.dat()[index] = maxFull;
  
  // set the top and base for convection

  double convBase = _missingFl32;
  double convTop = _missingFl32;
  double stratBase = _missingFl32;
  double stratTop = _missingFl32;
  
  for (size_t ii = 0; ii < textureProfile.size(); ii++) {
    double ht = _zKm[ii];
    fl32 texture = textureProfile[ii];
    if (texture == _missingFl32) {
      continue;
    }
    if (texture >= _minTextureForConvection) {
      if (convBase == _missingFl32) {
        convBase = ht;
      }
      convTop = ht;
    } else {
      if (stratBase == _missingFl32) {
        stratBase = ht;
      }
      stratTop = ht;
    }
  } // ii

  if (convBase != _missingFl32 && convTop != _missingFl32) {
    _convBaseKm.dat()[index] = convBase;
    _convTopKm.dat()[index] = convTop;
  }
  
  if (stratBase != _missingFl32 && stratTop != _missingFl32) {
    _stratBaseKm.dat()[index] = stratBase;
    _stratTopKm.dat()[index] = stratTop;
  }
  
}

/////////////////////////////////////////////////////////
// finalize the convective/stratiform partition

void ConvStratFinder::_finalizePartition()
  
{
  
  PMU_auto_register("ConvStratFinder::_finalizePartition()");

  // get data pointers
  
  ui08 *partition = _partition.dat();
  ui08 *partLow = _partitionLow.dat();
  ui08 *partMid = _partitionMid.dat();
  ui08 *partHigh = _partitionHigh.dat();
  
  // expand the convective area around the points
  // that have been identified
  
  size_t index = 0;
  for (size_t iy = 0; iy < _ny; iy++) {
    for (size_t ix = 0; ix < _nx; ix++, index++) {

      if (partition[index] == CATEGORY_CONVECTIVE) {
        _expandConvection(partition, ix, iy, index);
      }

      if (partLow[index] == CATEGORY_CONVECTIVE) {
        _expandConvection(partLow, ix, iy, index);
      }

      if (partMid[index] == CATEGORY_CONVECTIVE) {
        _expandConvection(partMid, ix, iy, index);
      }

      if (partHigh[index] == CATEGORY_CONVECTIVE) {
        _expandConvection(partHigh, ix, iy, index);
      }
      
    } // ix
  } // iy

  // load up the partitioned dbz arrays

  const fl32 *dbz = _dbz3D.dat();
  fl32 *convDbz = _convDbz.dat();
  fl32 *stratDbz = _stratDbz.dat();

  for (size_t ii = 0; ii < _nxyz; ii++) {
    convDbz[ii] = _missingFl32;
    stratDbz[ii] = _missingFl32;
  }

  for (size_t iy = 0, ii = 0; iy < _ny; iy++) {
    for (size_t ix = 0; ix < _nx; ix++, ii++) {
      if (partition[ii] == CATEGORY_CONVECTIVE) {
        size_t jj = ii;
        for (size_t iz = 0; iz < (size_t) _zKm.size(); iz++, jj += _nxy) {
          convDbz[jj] = dbz[jj];
        } // iz
      } else if (partition[ii] == CATEGORY_STRATIFORM) {
        size_t jj = ii;
        for (size_t iz = 0; iz < (size_t) _zKm.size(); iz++, jj += _nxy) {
          stratDbz[jj] = dbz[jj];
        } // iz
      }
    } // ix
  } // iy

}

////////////////////////////////////////////////////////////
// set partition, expanding convection by computed radius

void ConvStratFinder::_expandConvection(ui08 *partition,
                                        size_t ix,
                                        size_t iy,
                                        size_t index)
  
{
  
  // get the convetive radius for this point
  
  double backgroundDbz = _backgroundDbz.dat()[index];
  double radius = _computeConvRadiusKm(backgroundDbz);
  _convRadiusKm.dat()[index] = radius;
  double radSq = radius * radius;
  size_t ny = (size_t) floor(radius / _dy + 0.5);
  size_t nx = (size_t) floor(radius / _dx + 0.5);

  for (size_t jy = -ny; jy <= ny; jy++) {
    double yy = jy * _dy;
    for (size_t jx = -nx; jx <= nx; jx++) {
      double xx = jx * _dx;
      double rSq = yy * yy + xx * xx;
      if (rSq <= radSq) {
        size_t ky = iy + jy;
        size_t kx = ix + jx;
        if (ky >= 0 && ky < _ny - 1 &&
            kx >= 0 && kx < _nx - 1) {
          size_t kk = kx + ky * _nx;
          partition[kk] = CATEGORY_CONVECTIVE;
        } // if (ky >= 0 ...
      } // if (rSq <= radSq)
    } // jx
  } // jy

}

//////////////////////////////////////
// compute the kernels for this grid

void ConvStratFinder::_computeKernels()

{

  PMU_auto_register("ConvStratFinder::_computeKernels()");
    
  // texture kernel

  _textureKernelOffsets.clear();

  _nyTexture = (size_t) floor(_textureRadiusKm / _dy + 0.5);
  _nxTexture = (size_t) floor(_textureRadiusKm / _dx + 0.5);
  
  if (_verbose) {
    cerr << "Texture kernel size:" << endl;
    cerr << "  ny: " << _nyTexture << endl;
    cerr << "  nx: " << _nxTexture << endl;
    cerr << "  _dy: " << _dy << endl;
    cerr << "  _dx: " << _dx << endl;
  }

  kernel_t entry;
  for (int jdy = -_nyTexture; jdy <= _nyTexture; jdy++) {
    double yy = jdy * _dy;
    for (int jdx = -_nxTexture; jdx <= _nxTexture; jdx++) {
      double xx = jdx * _dx;
      double radius = sqrt(yy * yy + xx * xx);
      if (radius <= _textureRadiusKm) {
        entry.xx = xx;
        entry.yy = yy;
        entry.offset = jdx + jdy * _nx;
        _textureKernelOffsets.push_back(entry);
      }
    }
  }

  // background kernel

  _backgroundKernelOffsets.clear();
  
  _nyBackground = (size_t) floor(_backgroundRadiusKm / _dy + 0.5);
  _nxBackground = (size_t) floor(_backgroundRadiusKm / _dx + 0.5);
  
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
  out << "  _maxTextureForStratiform: " << _maxTextureForStratiform << endl;

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
  if (backgroundDbz == _missingFl32) {
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

  for (size_t ii = 0; ii < _nxy; ii++) {
    backgroundDbz[ii] = _missingFl32;
  }

  // background is mean dbz in a circle around a point
  
  for (size_t iy = _nyBackground; iy < _ny - _nyBackground; iy++) {
    
    size_t icenter = _nxBackground + iy * _nx;
    
    for (size_t ix = _nxBackground; ix < _nx - _nxBackground; ix++, icenter++) {
      
      double nn = 0.0;
      double sum = 0.0;
      
      for (size_t ii = 0; ii < _backgroundKernelOffsets.size(); ii++) {
        size_t kk = icenter + _backgroundKernelOffsets[ii];
        double dbz = colMaxDbz[kk];
        if (dbz != _missingFl32) {
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

ConvStratFinder::ComputeTexture::ComputeTexture(size_t iz) :
        TaThread(),
        _iz(iz)
{
  char name[128];
  sprintf(name, "ComputeTexture-level-%ld", _iz);
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

  for (size_t ii = 0; ii < _nx * _ny; ii++) {
    _texture[ii] = _missingVal;
  }
  
  // compute texture at each point in the plane

  size_t minPtsForTexture = 
    (size_t) (_minValidFraction * _kernelOffsets.size() + 0.5);
  PlaneFit pfit;
  
  for (int iy = _nyTexture; iy < (int) _ny - _nyTexture; iy++) {
    
    int icenter = _nxTexture + iy * _nx;
    
    for (int ix = _nxTexture; ix < (int) _nx - _nxTexture; ix++, icenter++) {
      
      if (_fractionCovered[icenter] < _minValidFraction) {
        continue;
      }
      if (_dbzColMax[icenter] == _missingVal) {
        continue;
      }

      // fit a plane to the reflectivity in a circular kernel around point
      
      pfit.clear();
      size_t count = 0;
      vector<double> dbzVals;
      vector<double> xx, yy;
      for (size_t ii = 0; ii < _kernelOffsets.size(); ii++) {
        const kernel_t &kern = _kernelOffsets[ii];
        size_t kk = icenter + kern.offset;
        double val = _dbz[kk];
        if (val != _missingVal) {
          pfit.addPoint(kern.xx, kern.yy, val);
          dbzVals.push_back(val);
          xx.push_back(kern.xx);
          yy.push_back(kern.yy);
          count++;
        }
      } // ii

      // check we have sufficient data around this point
      
      if (count >= minPtsForTexture) {

        // fit a plane to the reflectivity
        if (pfit.performFit() == 0) {
          // subtract plane fit from dbz values to
          // remove 2d trends in the data
          double aa = pfit.getCoeffA();
          double bb = pfit.getCoeffB();
          for (size_t ii = 0; ii < dbzVals.size(); ii++) {
            double delta = aa * xx[ii] + bb * yy[ii];
            dbzVals[ii] -= delta;
          }
        }

        // compute sdev of dbz squared
        
        double nn = 0.0;
        double sum = 0.0;
        double sumSq = 0.0;
        for (size_t ii = 0; ii < dbzVals.size(); ii++) {
          double val = dbzVals[ii];
          // constrain to positive values
          if (val < 1.0) {
            val = 1.0;
          }
          double dbzSq = val * val;
          sum += dbzSq;
          sumSq += dbzSq * dbzSq;
          nn++;
        } // ii
        double mean = sum / nn;
        double var = sumSq / nn - (mean * mean);
        if (var < 0.0) {
          var = 0.0;
        }
        double sdev = sqrt(var);
        _texture[icenter] = sqrt(sdev);

      } // if (count >= minPtsForTexture)
      
    } // ix
    
  } // iy
  
}

