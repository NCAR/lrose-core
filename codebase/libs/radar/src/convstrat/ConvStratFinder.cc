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
#include <map>
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

  _textureRadiusKm = 7.0;
  _minValidFractionForTexture = 0.25; 
  _minValidFractionForFit = 0.67; 
  _minVolForConvectiveKm3 = 20.0; 
  _minVertExtentForConvectiveKm = 1.0; 

  _minConvectivityForConvective = 0.5;
  _secondaryConvectivity = 0.7;
  _maxConvectivityForStratiform = 0.4;
  _minOverlapForClumping = 1;

  _shallowHtKm = 5.0;
  _deepHtKm = 10.0;

  _textureLimitLow = 0.0;
  _textureLimitHigh = 30.0;

  _dbzForEchoTops = 18.0;

  _nx = _ny = 0;
  _dx = _dy = 0.0;
  _dxKm = _dyKm = 0.0;
  _minx = _miny = 0;
  _nxy = _nxyz = 0;
  _projIsLatLon = false;
  _gridSet = false;


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
  _gridSet = true;

  // set geometry in km
  
  if (_projIsLatLon) {
    double meanLat = (_miny + _ny * _dy / 2.0);
    double cosLat = cos(meanLat * DEG_TO_RAD);
    _dyKm = _dy * KM_PER_DEG_AT_EQ;
    _dxKm = _dx * KM_PER_DEG_AT_EQ * cosLat;
  } else {
    _dxKm = _dx;
    _dyKm = _dy;
  }

  // allocate the arrays and set to missing

  _allocArrays();
  _initToMissing();

  // initialize height arrays

  setConstantHtThresholds(_shallowHtKm, _deepHtKm);

}

////////////////////////////////////////////////////////////////////
// Set the freezing level and divergence level as km MSL values

void ConvStratFinder::setConstantHtThresholds(double shallowHtKm,
                                              double deepHtKm)

{

  assert(_gridSet);

  _shallowHtKm = shallowHtKm;
  _deepHtKm = deepHtKm;
  
  fl32 *shallowHtArray = _shallowHtGrid.dat();
  fl32 *deepHtArray = _deepHtGrid.dat();
  
  for (size_t ii = 0; ii < _nxy; ii++) {
    shallowHtArray[ii] = _shallowHtKm;
    deepHtArray[ii] = _deepHtKm;
  }
  
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
  
  PMU_auto_register("ConvStratFinder::partition()");

  assert(_gridSet);

  // 2D case

  if (_zKm.size() == 1) {
    return _computePartition2D(dbz, dbzMissingVal);
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
  
  // compute spatial texture of reflectivity
  
  _computeTexture();

  // compute convectivity convectivity
  
  _computeConvectivity();

  // perform clumping on the convectivity field

  _performClumping();

  // set the 3D version of the partition

  if (_zKm.size() > 1) {
    _setPartition3D();
  } else {
    _setPartition2D();
  }

  // compute the 2D fields from the 3D fields

  if (_zKm.size() > 1) {
    _set2DFields();
  }

  return 0;

}

//////////////////////////////////////////////////
// Compute the partition in the 2D case

int ConvStratFinder::_computePartition2D(const fl32 *dbz, 
                                         fl32 dbzMissingVal)
{

  // z indices are 0

  _minIz = 0;
  _maxIz = 0;

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
  
  // compute column maxima and fraction covered
  // even though this is trivial for 2D data, it still must
  // be called to set certain arrays

  _computeColMax();
  
  // compute spatial texture of reflectivity
  
  _computeTexture();

  // compute convectivity
  
  _computeConvectivity();

  // set the partition

  _setPartition2D();

  return 0;

}

/////////////////////////////////////////////////////////
// allocate the arrays

void ConvStratFinder::_allocArrays()
  
{

  _dbz3D.alloc(_nxyz);
  _shallowHtGrid.alloc(_nxy);
  _deepHtGrid.alloc(_nxy);
  _colMaxDbz.alloc(_nxy);
  _fractionActive.alloc(_nxy);

  _partition3D.alloc(_nxyz);
  _partitionColMax.alloc(_nxy);
  _partition2D.alloc(_nxy);
  _convDbz.alloc(_nxyz);

  _texture3D.alloc(_nxyz);
  _textureColMax.alloc(_nxy);
  _texture2D.alloc(_nxy);

  _convectivity3D.alloc(_nxyz);
  _convectivityColMax.alloc(_nxy);
  _convectivity2D.alloc(_nxy);

  _convTopKm.alloc(_nxy);
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
  _initToMissing(_colMaxDbz, _missingFl32);
  _initToMissing(_fractionActive, _missingFl32);

  _initToMissing(_partition3D, _missingUi08);
  _initToMissing(_partitionColMax, _missingUi08);
  _initToMissing(_partition2D, _missingUi08);
  _initToMissing(_convDbz, _missingFl32);

  _initToMissing(_texture3D, _missingFl32);
  _initToMissing(_textureColMax, _missingFl32);
  _initToMissing(_texture2D, _missingFl32);

  _initToMissing(_convectivity3D, _missingFl32);
  _initToMissing(_convectivityColMax, _missingFl32);
  _initToMissing(_convectivity2D, _missingFl32);

  _initToMissing(_convTopKm, _missingFl32);
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
  _colMaxDbz.free();
  _fractionActive.free();

  _partition3D.free();
  _partitionColMax.free();
  _partition2D.free();
  _convDbz.free();

  _texture3D.free();
  _textureColMax.free();
  _texture2D.free();

  _convectivity3D.free();
  _convectivityColMax.free();
  _convectivity2D.free();

  _convTopKm.free();
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
      size_t xycenter = ix + iy * _nx;
      double count = 0;
      for (size_t ii = 0; ii < _textureKernelOffsets.size(); ii++) {
        size_t jj = xycenter + _textureKernelOffsets[ii].offset;
        double val = colMaxDbz[jj];
        if (val >= _minValidDbz) {
          count++;
        }
      } // ii
      double fraction = count / _textureKernelOffsets.size();
      fractionTexture[xycenter] = fraction;
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
  vector<ComputeTexture *> threads;
  for (size_t iz = _minIz; iz <= _maxIz; iz++) {
    size_t zoffset = iz * _nxy;
    ComputeTexture *thread = new ComputeTexture(iz);
    thread->setGridSize(_nx, _ny);
    thread->setKernelSize(_nxTexture, _nyTexture);
    thread->setMinValidFractionForTexture(_minValidFractionForTexture);
    thread->setMinValidFractionForFit(_minValidFractionForFit);
    thread->setDbz(dbz + zoffset, _missingFl32);
    thread->setFractionCovered(fractionTexture);
    thread->setKernelOffsets(_textureKernelOffsets);
    thread->setTextureArray(volTexture + zoffset);
    threads.push_back(thread);
  }

  // set threads going

  for (size_t ii = 0; ii < threads.size(); ii++) {
    if (_verbose) {
      cerr << "====>> starting texture thread: " << ii << endl;
    }
    threads[ii]->signalRunToStart();
  }

  // for 3D data compute the col max texture too
  // insert this at the same time as the running threads

  if (_zKm.size() > 1) {
    _computeTextureColMax();
  }

  // wait until they are done

  for (size_t ii = 0; ii < threads.size(); ii++) {
    threads[ii]->waitForRunToComplete();
    if (_verbose) {
      cerr << "====>> texture thread complete: " << ii << endl;
    }
  }

  // delete threads

  for (size_t ii = 0; ii < threads.size(); ii++) {
    if (_verbose) {
      cerr << "====>> deleting texture thread: " << ii << endl;
    }
    delete threads[ii];
  }

  threads.clear();
  if (_verbose) {
    cerr << "====>> All threads freed" << endl;
  }

}

/////////////////////////////////////////////////////////
// compute the spatial texture for the column max dbz

void ConvStratFinder::_computeTextureColMax()
  
{

  PMU_auto_register("ConvStratFinder::_computeTextureColMax()");

  // array pointers
  
  const fl32 *dbz = _colMaxDbz.dat();
  fl32 *texture = _textureColMax.dat();
  fl32 *fractionTexture = _fractionActive.dat();
  
  // initialize
  
  for (size_t ii = 0; ii < _nxy; ii++) {
    texture[ii] = _missingFl32;
  }
  
  // set up threads for computing texture at each level

  ComputeTexture thread(-1);
  thread.setGridSize(_nx, _ny);
  thread.setKernelSize(_nxTexture, _nyTexture);
  thread.setMinValidFractionForTexture(_minValidFractionForTexture);
  thread.setMinValidFractionForFit(_minValidFractionForFit);
  thread.setDbz(dbz, _missingFl32);
  thread.setFractionCovered(fractionTexture);
  thread.setKernelOffsets(_textureKernelOffsets);
  thread.setTextureArray(texture);

  // set thread going
  
  if (_verbose) {
    cerr << "====>> starting col max texture thread: " << endl;
  }
  thread.signalRunToStart();

  // wait until thread is done

  thread.waitForRunToComplete();
  if (_verbose) {
    cerr << "====>> col max texture thread complete: " << endl;
  }

}

/////////////////////////////////////////////////////////
// compute the convectivity convectivity

void ConvStratFinder::_computeConvectivity()
  
{

  // array pointers

  fl32 *texture3D = _texture3D.dat();
  fl32 *convectivity3D = _convectivity3D.dat();
  fl32 *active2D = _fractionActive.dat();
  
  // loop through the vol
  
  size_t index3D = 0;
  double textureRange = _textureLimitHigh - _textureLimitLow;
  double convectivitySlope = 1.0 / textureRange;
  
  for (size_t iz = 0; iz < _zKm.size(); iz++) {
    
    // loop through a plane
    
    size_t index2D = 0;
    
    for (size_t iy = 0; iy < _ny; iy++) {
      for (size_t ix = 0; ix < _nx; ix++, index2D++, index3D++) {
        
        fl32 convectivity = _missingFl32;
        if (active2D[index2D] >= _minValidFractionForTexture) {
          double texture = texture3D[index3D];
          if (texture < _textureLimitLow) {
            convectivity = _missingFl32;
          } else if (texture > _textureLimitHigh) {
            convectivity = 1.0;
          } else {
            convectivity = (texture - _textureLimitLow) * convectivitySlope;
          }
        }
        convectivity3D[index3D] = convectivity;

      } // ix
    } // iy
  } // iz

  // compute colmax convectivity

  fl32 *textureColMax = _textureColMax.dat();
  fl32 *convectivityColMax = _convectivityColMax.dat();
  size_t indexColMax = 0;

  for (size_t iy = 0; iy < _ny; iy++) {
    for (size_t ix = 0; ix < _nx; ix++, indexColMax++) {
      
      fl32 convectivity = _missingFl32;
      if (active2D[indexColMax] >= _minValidFractionForTexture) {
        double texture = textureColMax[indexColMax];
        if (texture < _textureLimitLow) {
          convectivity = _missingFl32;
        } else if (texture > _textureLimitHigh) {
          convectivity = 1.0;
        } else {
          convectivity = (texture - _textureLimitLow) * convectivitySlope;
        }
      }
      convectivityColMax[indexColMax] = convectivity;
      
    } // ix
  } // iy
  
}

/////////////////////////////////////////////////////////
// perform clumping on the convectivity field

void ConvStratFinder::_performClumping()
  
{

  // set up grid geometry
  
  PjgGridGeom gridGeom;
  gridGeom.setNx(_nx);
  gridGeom.setNy(_ny);
  gridGeom.setDx(_dx);
  gridGeom.setDy(_dy);
  gridGeom.setMinx(_minx);
  gridGeom.setMiny(_miny);
  gridGeom.setZKm(_zKm);
  gridGeom.setIsLatLon(_projIsLatLon);
  if (_projIsLatLon) {
    gridGeom.setProjType(PjgTypes::PROJ_LATLON);
  } else {
    gridGeom.setProjType(PjgTypes::PROJ_FLAT);
  }

  double minFractionAllParts = 0.5;
  double minFractionEachPart = 0.05;
  double minAreaEachPart = 20.0;

  _clumping.setUseDualThresholds(_secondaryConvectivity,
                                 minFractionAllParts,
                                 minFractionEachPart,
                                 minAreaEachPart,
                                 _minVolForConvectiveKm3,
                                 1.0e99,
                                 _verbose);
  
  vector<ClumpProps> clumpVec;
  _clumping.loadClumpVector(gridGeom, _convectivity3D.dat(), 
                            _minConvectivityForConvective,
                            _minOverlapForClumping,
                            clumpVec);

  if (_verbose) {
    cerr << "ConvStratFinder::_performClumping()" << endl;
    cerr << "  N clumps: " << clumpVec.size() << endl;
  }

  _freeClumps();
  for (size_t ii = 0; ii < clumpVec.size(); ii++) {
    StormClump *clump = new StormClump(this, clumpVec[ii]);
    clump->computeGeom();
    _clumps.push_back(clump);
  }

}

/////////////////////////////////////////////////////////
// free up clumps

void ConvStratFinder::_freeClumps()
  
{
  for (size_t ii = 0; ii < _clumps.size(); ii++) {
    delete _clumps[ii];
  }
  _clumps.clear();
}

/////////////////////////////////////////////////////////
// set 3d partition array

void ConvStratFinder::_setPartition3D()
  
{

  // loop through the convective clumps, setting the category
  
  for (size_t ii = 0; ii < _clumps.size(); ii++) {
    _clumps[ii]->setPartition();
  }
  
  // set the stratiform categories

  ui08 *partition3D = _partition3D.dat();
  fl32 *convectivity3D = _convectivity3D.dat();
  const fl32 *dbz3D = _dbz3D.dat();
  fl32 *convDbz = _convDbz.dat();
  const fl32 *shallowHtGrid = _shallowHtGrid.dat();
  const fl32 *deepHtGrid = _deepHtGrid.dat();

  // loop through (x,y)

  int nPtsPlane = _nx * _ny;
  
  for (size_t ix = 0; ix < _nx; ix++) {
    for (size_t iy = 0; iy < _ny; iy++) {

      int offset2D = iy * _nx + ix;
      fl32 shallowHtKm = shallowHtGrid[offset2D];
      fl32 deepHtKm = deepHtGrid[offset2D];
      
      // loop through the planes, accumulating layer info
  
      for (size_t iz = 0; iz < _zKm.size(); iz++) {
        
        int offset3D = iz * nPtsPlane + offset2D;

        // check if we have already assigned a convective category
        
        if (partition3D[offset3D] != CATEGORY_MISSING) {
          // set the convective dbz
          convDbz[offset3D] = dbz3D[offset3D];
          continue;
        }

        // check if there no convectivity at this point
        
        if (convectivity3D[offset3D] == _missingFl32) {
          continue;
        }
        if (convectivity3D[offset3D] == 0) {
          continue;
        }

        // is this mixed?
        
        if (convectivity3D[offset3D] > _maxConvectivityForStratiform) {
          partition3D[offset3D] = CATEGORY_MIXED;
          continue;
        }

        // assign a height-based stratiform category
        
        double zKm = _zKm[iz];
        if (zKm <= shallowHtKm) {
          partition3D[offset3D] = CATEGORY_STRATIFORM_LOW;
        } else if (zKm >= deepHtKm) {
          partition3D[offset3D] = CATEGORY_STRATIFORM_HIGH;
        } else {
          partition3D[offset3D] = CATEGORY_STRATIFORM_MID;
        }

      } // iz
    } // iy
  } // ix

  // set the partition for the col max

  _setPartitionColMax();

}

/////////////////////////////////////////////////////////
// set partition array for col max

void ConvStratFinder::_setPartitionColMax()
  
{
  
  const fl32 *convectivityColMax = _convectivityColMax.dat();
  ui08 *partitionColMax = _partitionColMax.dat();

  // loop through (x,y)
  
  size_t offset = 0;
  for (size_t ix = 0; ix < _nx; ix++) {
    for (size_t iy = 0; iy < _ny; iy++, offset++) {
      
      // check if there no convectivity at this point
      
      if (convectivityColMax[offset] == _missingFl32) {
        continue;
      }
      if (convectivityColMax[offset] == 0) {
        continue;
      }
      
      // is this mixed?
      
      ui08 part = CATEGORY_UNKNOWN;
      
      if (convectivityColMax[offset] <= _maxConvectivityForStratiform) {
        part = CATEGORY_STRATIFORM;
      } else if (convectivityColMax[offset] >= _minConvectivityForConvective) {
        part = CATEGORY_CONVECTIVE;
      } else {
        part = CATEGORY_MIXED;
      }
      
      partitionColMax[offset] = part;

    } // iy
  } // ix

}

/////////////////////////////////////////////////////////
// set 2d partition array - for 2D data
// set the stratiform categories
// and copy 3D fields to 2D fields

void ConvStratFinder::_setPartition2D()
  
{
  
  const fl32 *texture3D = _texture3D.dat();
  fl32 *texture2D = _texture2D.dat();
  const fl32 *convectivity3D = _convectivity3D.dat();
  fl32 *convectivity2D = _convectivity2D.dat();
  ui08 *partition3D = _partition3D.dat();
  ui08 *partition2D = _partition2D.dat();
  const fl32 *dbz3D = _dbz3D.dat();
  fl32 *convDbz = _convDbz.dat();

  // loop through (x,y)
  
  size_t offset = 0;
  for (size_t ix = 0; ix < _nx; ix++) {
    for (size_t iy = 0; iy < _ny; iy++, offset++) {
      
      texture2D[offset] = texture3D[offset];
      convectivity2D[offset] = convectivity3D[offset];
      convDbz[offset] = dbz3D[offset];
      
      // check if there no convectivity at this point
      
      if (convectivity3D[offset] == _missingFl32) {
        continue;
      }
      if (convectivity3D[offset] == 0) {
        continue;
      }
      
      // is this mixed?

      ui08 part = CATEGORY_UNKNOWN;
      
      if (convectivity3D[offset] <= _maxConvectivityForStratiform) {
        part = CATEGORY_STRATIFORM;
      } else if (convectivity3D[offset] >= _minConvectivityForConvective) {
        part = CATEGORY_CONVECTIVE;
      } else {
        part = CATEGORY_MIXED;
      }
      
      partition3D[offset] = part;
      partition2D[offset] = part;

    } // iy
  } // ix

}

/////////////////////////////////////////////////////////
// compute 2D summary fields

void ConvStratFinder::_set2DFields()
  
{
  
  PMU_auto_register("ConvStratFinder::_set2DFields()");

  // get data pointers
  
  ui08 *partition3D = _partition3D.dat();
  ui08 *partition2D = _partition2D.dat();
  fl32 *texture3D = _texture3D.dat();
  fl32 *texture2D = _texture2D.dat();
  fl32 *convectivity3D = _convectivity3D.dat();
  fl32 *convectivity2D = _convectivity2D.dat();
  fl32 *fractionActive = _fractionActive.dat();
  fl32 *convTopKm = _convTopKm.dat();
  fl32 *stratTopKm = _stratTopKm.dat();

  // loop through the x/y arrays
  
  size_t xycenter = 0;
  for (size_t iy = 0; iy < _ny; iy++) {
    for (size_t ix = 0; ix < _nx; ix++, xycenter++) {
      
      // check for activity

      if (fractionActive[xycenter] < _minValidFractionForTexture) {
        continue;
      }
      
      // init

      int pMax = 0;
      fl32 tMax = _missingFl32;
      fl32 cMax = _missingFl32;
      fl32 cTop = _missingFl32;
      fl32 sTop = _missingFl32;
        
      // loop through the z layers
      
      size_t zcenter = xycenter + _minIz * _nxy;
      for (size_t iz = _minIz; iz <= _maxIz; iz++, zcenter += _nxy) {

        fl32 zKm = _zKm[iz];
        
        int p3D = partition3D[zcenter];
        if (p3D > pMax) {
          pMax = p3D;
        }
        if (p3D >= CATEGORY_STRATIFORM_LOW &&
            p3D <= CATEGORY_STRATIFORM_HIGH) {
          sTop = zKm;
        } else if (p3D >= CATEGORY_CONVECTIVE_SHALLOW &&
                   p3D <= CATEGORY_CONVECTIVE_DEEP) {
          cTop = zKm;
        }

        fl32 t3D = texture3D[zcenter];
        if (t3D > tMax) {
          tMax = t3D;
        }

        fl32 c3D = convectivity3D[zcenter];
        if (c3D > cMax) {
          cMax = c3D;
        }

      } // iz

      partition2D[xycenter] = pMax;
      texture2D[xycenter] = tMax;
      convectivity2D[xycenter] = cMax;
      convTopKm[xycenter] = cTop;
      stratTopKm[xycenter] = sTop;

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

  _nyTexture = (size_t) floor(_textureRadiusKm / _dyKm + 0.5);
  _nxTexture = (size_t) floor(_textureRadiusKm / _dxKm + 0.5);
  
  if (_verbose) {
    cerr << "Texture kernel size:" << endl;
    cerr << "  ny: " << _nyTexture << endl;
    cerr << "  nx: " << _nxTexture << endl;
    cerr << "  _dyKm: " << _dyKm << endl;
    cerr << "  _dxKm: " << _dxKm << endl;
  }

  kernel_t entry;
  for (int jdy = -_nyTexture; jdy <= _nyTexture; jdy++) {
    double yy = jdy * _dyKm;
    for (int jdx = -_nxTexture; jdx <= _nxTexture; jdx++) {
      double xx = jdx * _dxKm;
      double radius = sqrt(yy * yy + xx * xx);
      if (radius <= _textureRadiusKm) {
        entry.jx = jdx;
        entry.jy = jdy;
        entry.xx = xx;
        entry.yy = yy;
        entry.offset = jdx + jdy * _nx;
        _textureKernelOffsets.push_back(entry);
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
  out << "  _textureRadiusKm: " << _textureRadiusKm << endl;
  out << "  _minValidFractionForTexture: " << _minValidFractionForTexture << endl;
  out << "  _maxConvectivityForStratiform: " << _maxConvectivityForStratiform << endl;
  out << "  _minConvectivityForConvective: " << _minConvectivityForConvective << endl;
  out << "  _minOverlapForClumping: " << _minOverlapForClumping << endl;

  out << "  _nx: " << _nx << endl;
  out << "  _ny: " << _ny << endl;
  out << "  _minx: " << _minx << endl;
  out << "  _miny: " << _miny << endl;
  out << "  _dxKm: " << _dxKm << endl;
  out << "  _dyKm: " << _dyKm << endl;

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
  if (iz < 0) {
    sprintf(name, "ComputeTexture-column-max");
  } else {
    sprintf(name, "ComputeTexture-level-%ld", _iz);
  }
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
    (size_t) (_minValidFractionForTexture * _kernelOffsets.size() + 0.5);
  size_t minPtsForFit = 
    (size_t) (_minValidFractionForFit * _kernelOffsets.size() + 0.5);

  PlaneFit pfit;
  
  for (int iy = _nyTexture; iy < (int) _ny - _nyTexture; iy++) {
    
    int icenter = _nxTexture + iy * _nx;
    
    for (int ix = _nxTexture; ix < (int) _nx - _nxTexture; ix++, icenter++) {
      
      if (_fractionCovered[icenter] < _minValidFractionForTexture) {
        continue;
      }
      if (_dbz[icenter] == _missingVal) {
        continue;
      }

      // fit a plane to the reflectivity in a circular kernel around point
      
      pfit.clear();
      size_t count = 0;
      vector<double> dbzVals;
      double sumDbz = 0.0;
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
          sumDbz += val;
          count++;
        }
      } // ii

      double meanDbz = sumDbz / count;
      meanDbz = max(meanDbz, 1.0);

      // check we have sufficient data around this point
      // for computing the fit
      
      if (count >= minPtsForFit) {
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
      } // if (count >= minPtsForFit)

      // check we have sufficient data around this point
      // for computing the texture
      
      if (count >= minPtsForTexture) {

        // compute sdev of dbz squared
        
        double nn = 0.0;
        double sum = 0.0;
        double sumSq = 0.0;
        for (size_t ii = 0; ii < dbzVals.size(); ii++) {
          double val = dbzVals[ii];
          // constrain to positive values
          val = max(val, 1.0);
          double dbzSq = val * val;
          sum += dbzSq;
          sumSq += dbzSq * dbzSq;
          nn++;
        } // ii
        // for missing points, substitute the mean
        if (dbzVals.size() < _kernelOffsets.size()) {
          double minSq = meanDbz * meanDbz;
          for (size_t ii = dbzVals.size(); ii < _kernelOffsets.size(); ii++) {
            sum += minSq;
            sumSq += minSq * minSq;
            nn++;
          }
        }
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

///////////////////////////////////////////////////////////////
// StormClump inner class
//
///////////////////////////////////////////////////////////////

// Constructor

ConvStratFinder::StormClump::StormClump(ConvStratFinder *finder,
                                        const ClumpProps &cprops) :
        _finder(finder),
        _cprops(cprops)
{
  _id = 0;
  _volumeKm3 = 0.0;
  _vertExtentKm = 0.0;
  _nPtsTotal = 0;
  _nPtsShallow = 0;
  _nPtsMid = 0;
  _nPtsDeep = 0;
}  

// destructor

ConvStratFinder::StormClump::~StormClump() 
{
}

// compute clump geom


void ConvStratFinder::StormClump::computeGeom() 
{

  // init
  
  _nPtsTotal = _cprops.nPoints3D();
  _volumeKm3 = 0.0;
  _vertExtentKm = 0.0;
  _nPtsShallow = 0;
  _nPtsMid = 0;
  _nPtsDeep = 0;

  // compute the volume, and number of points
  // in each height layer
  
  int nx = _finder->_nx;
  int nz = _finder->_zKm.size();
  
  const fl32 *shallowHtGrid = _finder->_shallowHtGrid.dat();
  const fl32 *deepHtGrid = _finder->_deepHtGrid.dat();

  double minZKm = 9999.0;
  double maxZKm = -9999.0;

  for (size_t irun = 0; irun < _cprops.intvGlobal().size(); irun++) {
    
    const Interval &intvl = _cprops.intvGlobal(irun);
    if (irun == 0) {
      _id = intvl.id;
    }
    
    int iz = intvl.plane;
    int iy = intvl.row_in_plane;

    double zKm = _finder->_zKm[iz];
    minZKm = min(zKm, minZKm);
    maxZKm = max(zKm, maxZKm);

    double dxKm = _finder->_dxKm;
    double dyKm = _finder->_dyKm;
    double dzKm = 0.0;
    if (iz == 0) {
      dzKm = _finder->_zKm[iz+1] - _finder->_zKm[iz];
    } else if (iz == nz - 1) {
      dzKm = _finder->_zKm[iz] - _finder->_zKm[iz-1];
    } else {
      dzKm = (_finder->_zKm[iz+1] - _finder->_zKm[iz-1]) / 2.0;
    }
    
    if (_finder->_projIsLatLon) {
      double latDeg = (_finder->_miny + _finder->_ny * _finder->_dy / 2.0);
      double cosLat = cos(latDeg * DEG_TO_RAD);
      dxKm = _finder->_dx * KM_PER_DEG_AT_EQ * cosLat;
    }

    double dVol = dxKm * dyKm * dzKm;
    int offset2D = iy * nx + intvl.begin;

    for (int ix = intvl.begin; ix <= intvl.end; ix++, offset2D++) {

      fl32 shallowHtKm = shallowHtGrid[offset2D];
      fl32 deepHtKm = deepHtGrid[offset2D];
      _volumeKm3 += dVol;

      if (zKm <= shallowHtKm) {
        _nPtsShallow++;
      } else if (zKm >= deepHtKm) {
        _nPtsDeep++;
      } else {
        _nPtsMid++;
      }

    } // ix
    
  } // irun

  _vertExtentKm = maxZKm - minZKm;

}

// Set the partition category based on clump properties

void ConvStratFinder::StormClump::setPartition() 
{

  // compute fraction in each height category
  
  double fracShallow = (double) _nPtsShallow / (double) _nPtsTotal;
  double fracDeep = (double) _nPtsDeep / (double) _nPtsTotal;

  // set the category
  
  int category = CATEGORY_MISSING;
  if (_volumeKm3 < _finder->_minVolForConvectiveKm3) {
    category = CATEGORY_MIXED;
  } else if (_vertExtentKm < _finder->_minVertExtentForConvectiveKm) {
    category = CATEGORY_MIXED;
  } else if (fracShallow < 0.05 && stratiformBelow()) {
    if (fracDeep > 0.75) {
      category = CATEGORY_MIXED;
    } else {
      category = CATEGORY_CONVECTIVE_ELEVATED;
    }
  } else if (fracShallow > 0.95) {
    category = CATEGORY_CONVECTIVE_SHALLOW;
  } else if (fracDeep > 0.05) {
    category = CATEGORY_CONVECTIVE_DEEP;
  } else {
    category = CATEGORY_CONVECTIVE_MID;
  }

  // compute the volume, and number of points
  // in each height layer
  
  ui08 *partition = _finder->_partition3D.dat();

  int nPtsPlane = _finder->_nx * _finder->_ny;
  int nx = _finder->_nx;
  
  for (size_t irun = 0; irun < _cprops.intvGlobal().size(); irun++) {
    
    const Interval &intvl = _cprops.intvGlobal(irun);
    
    int iy = intvl.row_in_plane;
    int iz = intvl.plane;
    
    int offset2D = iy * nx + intvl.begin;
    int offset3D = iz * nPtsPlane + offset2D;
    
    for (int ix = intvl.begin; ix <= intvl.end; ix++, offset3D++) {
      partition[offset3D] = category;
    } // ix
    
  } // irun

}

///////////////////////////////////////////////////////////
// Check for stratiform echo below

bool ConvStratFinder::StormClump::stratiformBelow() 
{

  // we go through each point in the clump, and check the point
  // immediately below. We count up misses and stratiform points
  // and then compute the fraction of stratiform points in
  // the total. If greater than 0.5, we say there is stratiform
  // below. Also, if any clump point is at the lowest point in the
  // grid, we return false since there is no room for stratiform
  // below.
  
  const fl32 *convectivity3D = _finder->_convectivity3D.dat();
  int nPtsPlane = _finder->_nx * _finder->_ny;
  int nx = _finder->_nx;

  double nMiss = 0.0;
  double nStrat = 0.0;
  
  for (size_t irun = 0; irun < _cprops.intvGlobal().size(); irun++) {
    
    const Interval &intvl = _cprops.intvGlobal(irun);
    
    int iy = intvl.row_in_plane;
    int iz = intvl.plane;
    if (iz == 0) {
      // clump is at lowest level
      // so no stratiform can be below
      return false;
    }
    
    // check grid points on plane below this one
    int offset2D = iy * nx + intvl.begin;
    int offset3D = (iz - 1) * nPtsPlane + offset2D;
    
    for (int ix = intvl.begin; ix <= intvl.end; ix++, offset3D++) {
      fl32 conv = convectivity3D[offset3D];
      if (conv == _missingFl32) {
        // point below convection is missing
        nMiss++;
      } else if (conv < _finder->_minConvectivityForConvective) {
        // point just below convection is stratiform
        nStrat++;
      }
    } // ix
    
  } // irun

  // compute the fraction of stratiform points below convective points

  double nTotal = nMiss + nStrat;
  double fractionStrat = nStrat / nTotal;

  // if fraction stratiform exceeds 0.5
  // we conclude we have stratiform below

  if (fractionStrat > 0.9) {
    return true;
  } else {
    return false;
  }

}

