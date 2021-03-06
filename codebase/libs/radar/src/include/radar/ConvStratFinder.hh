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
/////////////////////////////////////////////////////////////
// ConvStratFinder.hh
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

#ifndef ConvStratFinder_HH
#define ConvStratFinder_HH

#include <string>
#include <vector>
#include <toolsa/TaArray.hh>
#include <toolsa/TaThread.hh>
#include <dataport/port_types.h>
#include <euclid/GridClumping.hh>
using namespace std;

////////////////////////
// This class

class ConvStratFinder {
  
public:

  typedef enum {
    CATEGORY_MISSING = 99,
    CATEGORY_STRATIFORM_SHALLOW = 14,
    CATEGORY_STRATIFORM_MID = 15,
    CATEGORY_STRATIFORM_ELEVATED = 16,
    CATEGORY_STRATIFORM_DEEP = 17,
    CATEGORY_STRATIFORM = 19,
    CATEGORY_MIXED = 29,
    CATEGORY_CONVECTIVE_SMALL = 33,
    CATEGORY_CONVECTIVE_SHALLOW = 34,
    CATEGORY_CONVECTIVE_MID = 35,
    CATEGORY_CONVECTIVE_ELEVATED = 36,
    CATEGORY_CONVECTIVE_DEEP = 37,
    CATEGORY_CONVECTIVE = 39,
    CATEGORY_UNKNOWN
  } category_t;

  typedef struct {
    double xx;
    double yy;
    ssize_t offset;
  } kernel_t;
  
  // constructor
  
  ConvStratFinder();
  
  // destructor
  
  ~ConvStratFinder();

  // set algorithm parameters

  ////////////////////////////////////////////////////////////////////
  // Min height used in analysis (km).
  // Only data at or above this altitude is used.

  void setMinValidHtKm(double val) { _minValidHtKm = val; }

  ////////////////////////////////////////////////////////////////////
  // Max height used in analysis (km).
  // Only data at or below this altitude is used.

  void setMaxValidHtKm(double val) { _maxValidHtKm = val; }

  ////////////////////////////////////////////////////////////////////
  // Minimum reflectivity threshold for this analysis (dBZ).
  // Reflectivity below this threshold is set to missing.

  void setMinValidDbz(double val) { _minValidDbz = val; }

  // converting texture to convectivity interest
  // these are the limits mapping to 0 and 1
  
  void setTextureLimitLow(double val) { _textureLimitLow = val; }
  void setTextureLimitHigh(double val) { _textureLimitHigh = val; }
  
  // set interest threshold for convection

  void setConvInterestThreshold(double val) {
    _convInterestThreshold = val;
  }
  
  ////////////////////////////////////////////////////////////////////
  // Reflectivity value that indicates definite convection.  If the
  // reflectivity exceeds this value at a point, we assume convection
  // is definitely active at that point. To use this, we first compute
  // the column maximum reflectivity. If the column max dbz at a point
  // exceeds this threshold, then we flag that point as convective.

  void setDbzForDefiniteConvection(double val) {
    _dbzForDefiniteConvection = val; 
  }
  
  ////////////////////////////////////////////////////////////////////
  // Reflectivity threshold for echo tops (dBZ).
  // Echo tops are defined as the max ht with reflectivity at or
  // above this value.
  
  void setDbzForEchoTops(double val) { _dbzForEchoTops = val; }
  
  ////////////////////////////////////////////////////////////////////
  // Compute the radius of convective influence from the background
  // reflectivity.
  // Given definite convection at a point (see above),
  // we set all points within the computed radius to be convective.
  //
  //
  // minConvRadiusKm = min convective radius if computed
  // maxConvRadiusKm = max convective radius if computed
  // dbzForMinRadius = background dbz for min radius
  // dbzForMaxRadius = background dbz for max radius
  // backgroundRadiusKm = kernel radius for computing background dbz

  void setComputeConvRadius(double dbzForMinRadius,
                            double dbzForMaxRadius,
                            double minConvRadiusKm,
                            double maxConvRadiusKm,
                            double backgroundRadiusKm) {
    _minConvRadiusKm = minConvRadiusKm;
    _maxConvRadiusKm = maxConvRadiusKm;
    _dbzForMinRadius = dbzForMinRadius;
    _dbzForMaxRadius = dbzForMaxRadius;
    _backgroundRadiusKm = backgroundRadiusKm;
    _deltaRadius = _maxConvRadiusKm - _minConvRadiusKm;
    _deltaBackgroundDbz = _dbzForMaxRadius -_dbzForMinRadius;
    _radiusSlope = _deltaRadius / _deltaBackgroundDbz;
  }

  ////////////////////////////////////////////////////////////////////
  // Radius for texture analysis (km).  We determine the reflectivity
  // 'texture' at a point by computing the standard deviation of the
  // square of the reflectivity, for all grid points within this
  // radius of the central point. We then compute the square root of
  // that sdev.

  void setTextureRadiusKm(double val) { _textureRadiusKm = val; }

  ////////////////////////////////////////////////////////////////////
  // Minimum fraction of surroundingpoints for texture computations.
  // For a valid computation of texture, we require at least this
  // fraction of points around the central point to have reflectivity
  // in excess of min_valid_dbz.

  void setMinValidFractionForTexture(double val) {
    _minValidFractionForTexture = val; 
  }

  ////////////////////////////////////////////////////////////////////
  // Minimum texture for convection at a point.  If the texture at a
  // point exceeds this value, we set the convective flag at this
  // point. We then expand the convective influence around the point
  // using convetive_radius_km.

  void setMinTextureForConvection(double val) {
    _minTextureForConvection = val; 
  }

  ////////////////////////////////////////////////////////////////////
  // Minimum texture for convection at a point.  If the texture at a
  // point exceeds this value, we set the convective flag at this
  // point. We then expand the convective influence around the point
  // using convetive_radius_km.

  void setMaxTextureForStratiform(double val) {
    _maxTextureForStratiform = val; 
  }

  ////////////////////////////////////////////////////////////////////
  // Set grid details

  void setGrid(size_t nx, size_t ny,
               double dx, double dy,
               double minx, double miny,
               const vector<double> &zKm,
               bool projIsLatLon = false);
  
  ////////////////////////////////////////////////////////////////////
  // Set the shallow and deep height thresholds, in km.
  // This sets the heights to constant values.
  
  void setConstantHtThresholds(double shallowHtKm,
                               double deepHtKm);

  // Set the shallow and deep height thresholds as a grid
  // These are generally derived from model temperature
  // These must be on the same grid as the radar DBZ data
  
  void setGridHtThresholds(const fl32 *shallowHtGrid,
                           const fl32 *deepHtGrid,
                           size_t nptsPlane);
  
  ////////////////////////////////////////////////////////////////////
  // Set debugging to on or verbose

  void setDebug(bool state) { _debug = state; }

  void setVerbose(bool state) {
    _verbose = state;
    if (state) {
      _debug = true;
    }
  }
  
  ////////////////////////////////////////////////////////////////////
  // compute the partition
  // Returns 0 on success, -1 on failure

  int computePartition(const fl32 *dbz, fl32 dbzMissingVal);

  // get the input fields
  
  const fl32 *getDbz3D() const { return _dbz3D.dat(); }
  const fl32 *getShallowHtGrid() const { return _shallowHtGrid.dat(); }
  const fl32 *getDeepHtGrid() const { return _deepHtGrid.dat(); }
  
  ////////////////////////////////////////////////////////////////////
  // get the resulting partition
  // will be set to the relevant category

  const ui08 *getPartition3D() const { return _partition3D.dat(); }
  const ui08 *getPartition() const { return _partition.dat(); }
  const ui08 *getPartitionLow() const { return _partitionLow.dat(); }
  const ui08 *getPartitionMid() const { return _partitionMid.dat(); }
  const ui08 *getPartitionHigh() const { return _partitionHigh.dat(); }

  const fl32 *getConvectiveDbz() const { return _convDbz.dat(); }
  const fl32 *getStratiformDbz() const { return _stratDbz.dat(); }

  ////////////////////////////////////////////////////////////////////
  // get derived fields

  const fl32 *getTexture3D() const { return _texture3D.dat(); }
  const fl32 *getInterest3D() const { return _interest3D.dat(); }
  const fl32 *getMeanTexture() const { return _meanTexture.dat(); }
  const fl32 *getMeanTextureLow() const { return _meanTextureLow.dat(); }
  const fl32 *getMeanTextureMid() const { return _meanTextureMid.dat(); }
  const fl32 *getMeanTextureHigh() const { return _meanTextureHigh.dat(); }
  const fl32 *getMaxTexture() const { return _maxTexture.dat(); }
  const fl32 *getFractionActive() const { return _fractionActive.dat(); }
  const fl32 *getColMaxDbz() const { return _colMaxDbz.dat(); }
  const fl32 *getBackgroundDbz() const { return _backgroundDbz.dat(); }
  const fl32 *getConvRadiusKm() const { return _convRadiusKm.dat(); }

  const fl32 *getConvBaseKm() const { return _convBaseKm.dat(); }
  const fl32 *getConvTopKm() const { return _convTopKm.dat(); }

  const fl32 *getStratBaseKm() const { return _stratBaseKm.dat(); }
  const fl32 *getStratTopKm() const { return _stratTopKm.dat(); }

  const fl32 *getEchoTopKm() const { return _echoTopKm.dat(); }

  // get missing value for float arrays

  ui08 getMissingUi08() const { return _missingUi08; }
  fl32 getMissingFl32() const { return _missingFl32; }

  // get grid details

  size_t getGridNx() const { return _nx; }
  size_t getGridNy() const { return _ny; }

  double getGridMinx() const { return _minx; }
  double getGridMiny() const { return _miny; }

  double getGridDx() const { return _dx; }
  double getGridDy() const { return _dy; }
  
  const vector<double> &getZKm() const { return _zKm; }
  bool getProjIsLatlon() const { return _projIsLatLon; }

  // free up arrays when done, if you want to keep memory usage down

  void freeArrays();

protected:
  
private:

  // private data
  
  static const fl32 _missingFl32;
  static const ui08 _missingUi08;
  
  bool _debug; // Print debug messages
  bool _verbose; // Print verbose debug messages

  double _minValidHtKm;
  double _maxValidHtKm;
  double _minValidDbz;
  double _dbzForDefiniteConvection;
  double _textureRadiusKm;
  double _minValidFractionForTexture;
  double _minTextureForConvection;
  double _maxTextureForStratiform;

  double _minConvRadiusKm; // min convective radius if computed
  double _maxConvRadiusKm; // max convective radius if computed
  double _dbzForMinRadius; // background dbz for min radius
  double _dbzForMaxRadius; // background dbz for max radius
  double _deltaRadius;
  double _deltaBackgroundDbz;
  double _radiusSlope;
  double _backgroundRadiusKm; // radius for computing background dbz
  double _dbzForEchoTops; // reflectivity for storm tops

  vector<kernel_t> _textureKernelOffsets;
  vector<ssize_t> _backgroundKernelOffsets;

  // grid details

  bool _gridSet;
  
  size_t _nx, _ny;
  double _minx, _miny;
  double _dx, _dy;
  vector<double> _zKm;
  bool _projIsLatLon;

  size_t _nxy, _nxyz;
  size_t _minIz, _maxIz;
  
  int _nxTexture, _nyTexture;
  int _nxBackground, _nyBackground;

  // specify freezing level, and divergence level, by ht MSL
  // if this is false, grids for fz and div level must be passed in
  
  bool _specifyLevelsByHtValues;
  double _shallowHtKm;
  double _deepHtKm;

  // converting texture to convectivity interest
  // these are the limits mapping to 0 and 1
  
  double _textureLimitLow;
  double _textureLimitHigh;

  // interest threshold for convection

  double _convInterestThreshold;

  // clumping the convective regions
  
  GridClumping _clumping;
  int _nClumps;
  
  // inputs
  
  TaArray<fl32> _dbz3D;
  TaArray<fl32> _shallowHtGrid; // grid for shallow cloud ht threshold
  TaArray<fl32> _deepHtGrid;    // grid for deep cloud ht threshold
  
  // primary outputs
  
  TaArray<ui08> _partition3D;
  TaArray<ui08> _partition;
  TaArray<ui08> _partitionLow;
  TaArray<ui08> _partitionMid;
  TaArray<ui08> _partitionHigh;

  TaArray<fl32> _convDbz;
  TaArray<fl32> _stratDbz;
  
  // intermediate fields
  
  TaArray<fl32> _texture3D;
  TaArray<fl32> _interest3D;
  TaArray<fl32> _meanTexture;
  TaArray<fl32> _meanTextureLow;
  TaArray<fl32> _meanTextureMid;
  TaArray<fl32> _meanTextureHigh;
  TaArray<fl32> _maxTexture;
  TaArray<fl32> _fractionActive;
  TaArray<fl32> _colMaxDbz;
  TaArray<fl32> _backgroundDbz;
  TaArray<fl32> _convRadiusKm;

  TaArray<fl32> _convBaseKm;
  TaArray<fl32> _convTopKm;
  TaArray<fl32> _stratBaseKm;
  TaArray<fl32> _stratTopKm;
  TaArray<fl32> _echoTopKm;

  // methods
  
  void _allocArrays();
  void _initToMissing();
  void _initToMissing(TaArray<fl32> &array, fl32 missingVal);
  void _initToMissing(TaArray<ui08> &array, ui08 missingVal);
  void _computeColMax();
  void _finalizePartition();
  void _expandConvection(ui08 *partition,
                         size_t ix, size_t iy, size_t index);
  void _computeTexture();
  void _computeInterest();
  void _performClumping();
  void _setPartition3D();
  void _computeProps();
  void _computeProps(size_t index, vector<fl32> &textureProfile);
  void _computeKernels();
  void _printSettings(ostream &out);
  double _computeConvRadiusKm(double backgroundDbz);
  void _computeBackgroundDbz();
  
  // inner class for starting timers in a separate thread

  class ComputeTexture : public TaThread
  {  
    
  public:   
    
    // constructor saves _sd3c pointer
    
    ComputeTexture(size_t iz);
    
    // destructor
    
    virtual ~ComputeTexture();
    
    // set parameters
    
    void setGridSize(size_t nx, size_t ny)
    {
      _nx = nx;
      _ny = ny;
    }
    
    void setKernelSize(size_t nx, size_t ny)
    {
      _nxTexture = nx;
      _nyTexture = ny;
    }
    
    void setMinValidFraction(double val)
    {
      _minValidFraction = val;
    }
    
    void setDbz(const fl32 *dbz, const fl32 *dbzColMax,
                fl32 missingVal)
    {
      _dbz = dbz;
      _dbzColMax = dbzColMax;
      _missingVal = missingVal;
    }
    
    void setFractionCovered(const fl32 *frac)
    {
      _fractionCovered = frac;
    }
    
    void setKernelOffsets(const vector<kernel_t> &offsets)
    {
      _kernelOffsets = offsets;
    }
    
    void setTextureArray(fl32 *texture)
    {
      _texture = texture;
    }
    
    // override run method
    
    virtual void run();
    
  private:
    
    size_t _iz;
    size_t _nx, _ny;
    int _nxTexture, _nyTexture;
    double _minValidFraction;
    fl32 _missingVal;
    const fl32 *_dbz;
    const fl32 *_dbzColMax;
    const fl32 *_fractionCovered;
    fl32 *_texture;
    vector<kernel_t> _kernelOffsets;
    
  };

};

#endif
