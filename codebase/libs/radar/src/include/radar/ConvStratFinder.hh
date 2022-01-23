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
// ConvStratFinder echoTypes stratiform and convective regions in a
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
#include <euclid/ClumpingMgr.hh>
#include <euclid/ClumpProps.hh>
using namespace std;

////////////////////////
// This class

class ConvStratFinder {
  
public:

  typedef enum {
    CATEGORY_MISSING = 0,
    CATEGORY_STRATIFORM_LOW = 14,
    CATEGORY_STRATIFORM = 15,
    CATEGORY_STRATIFORM_MID = 16,
    CATEGORY_STRATIFORM_HIGH = 18,
    CATEGORY_MIXED = 25,
    CATEGORY_CONVECTIVE_ELEVATED = 32,
    CATEGORY_CONVECTIVE_SHALLOW = 34,
    CATEGORY_CONVECTIVE = 35,
    CATEGORY_CONVECTIVE_MID = 36,
    CATEGORY_CONVECTIVE_DEEP = 38,
    CATEGORY_UNKNOWN
  } category_t;

  typedef struct {
    int jx, jy;
    double xx;
    double yy;
    ssize_t offset;
  } kernel_t;

  // constructor
  
  ConvStratFinder();
  
  // destructor
  
  ~ConvStratFinder();

  ////////////////////////////////////////////////////////////////////
  // Set debugging to on or verbose

  void setDebug(bool val) { _debug = val; }

  void setVerbose(bool val) {
    _verbose = val;
    if (val) {
      _debug = true;
    }
  }

  ////////////////////////////////////////////////////////////////////
  // Set use of multiple threads, default is TRUE

  void setUseMultipleThreads(bool val)
  {
    _useMultipleThreads = val;
  }

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
  // Use the col max reflectivity for computing the texture,
  // instead of doing this on each plane.
  // This uses less CPU.
  
  void setUseDbzColMax(bool val) { _useDbzColMax = val; }
  
  ////////////////////////////////////////////////////////////////////
  // Minimum reflectivity threshold for this analysis (dBZ).
  // Reflectivity below this threshold is set to missing.
  // This selects the grid points we use for computing the active
  // fraction within the texture radius.

  void setMinValidDbz(double val) { _minValidDbz = val; }

  ////////////////////////////////////////////////////////////////////
  // Set DBZ base.
  // Before computing the texture, we subtract the baseDBZ
  // from the measured DBZ.
  // This adjusts the DBZ values into the positive range.
  // For S-, C- and X-band radars, this can be set to
  // 0 dBZ, which is the default.
  // For Ka-band radars this should be around -10 dBZ.
  // For W-band radars -20 dBZ is appropriate.
  
  void setBaseDbz(double val) { _baseDbz = val; }

  // converting texture to convectivity convectivity
  // these are the limits for mapping texture to convectivity from 0 and 1

  void setTextureLimitLow(double val) {
    _textureLimitLow = val;
  }
  void setTextureLimitHigh(double val) {
    _textureLimitHigh = val;
  }

  // set convectivity threshold for convective regions
  // convectivity values above this indicate convective
  
  void setMinConvectivityForConvective(double val) {
    _minConvectivityForConvective = val;
  }
  
  // set whether to use dual thresholds
  // this is for splitting clumps using a second convectivity threshold
  
  void setUseDualThresholds(double secondary_threshold,
                            double min_fraction_all_parts,
                            double min_fraction_each_part,
                            double min_size_each_part);

  // Set convectivity threshold for stratiform regions.
  // Convectivity values below this indicate stratiform.
  // Convectivity values between this and minConvectivityForConvective
  // indicate mixed.
  
  void setMaxConvectivityForStratiform(double val) {
    _maxConvectivityForStratiform = val;
  }
  
  // set minimum grid overlap for clumping the convective regions
  
  void setMinGridOverlapForClumping(int val) {
    _minOverlapForClumping = val;
  }
  
  ////////////////////////////////////////////////////////////////////
  // Radius for texture analysis (km).  We determine the reflectivity
  // 'texture' at a point by computing the standard deviation of the
  // square of the reflectivity, for all grid points within this
  // radius of the central point. We then compute the square root of
  // that sdev.

  void setTextureRadiusKm(double val) { _textureRadiusKm = val; }

  ////////////////////////////////////////////////////////////////////
  // Minimum fraction of surrounding points for planar fit.
  // For a valid computation of the 2D fit to reflectivity at a point,
  // we require at least this fraction of points around the central
  // point to have reflectivity in excess of min_valid_dbz.

  void setMinValidFractionForTexture(double val) {
    _minValidFractionForTexture = val; 
  }

  ////////////////////////////////////////////////////////////////////
  // Minimum fraction of surrounding points for texture computations.
  // For a valid computation of texture, we require at least this
  // fraction of points around the central point to have reflectivity
  // in excess of min_valid_dbz.

  void setMinValidFractionForFit(double val) {
    _minValidFractionForFit = val; 
  }

  ////////////////////////////////////////////////////////////////////
  // Set the minimum volume for convection (km3)
  // If a convective clump is smaller than this, the
  // clump is set to mixed
  
  void setMinVolForConvectiveKm3(double val) {
    _minVolForConvectiveKm3 = val; 
  }

  ////////////////////////////////////////////////////////////////////
  // Set the minimum vertical extent for convective echoes.
  // The vertical extent is computed as the mid height of the top layer
  // in the echo minus the mid height of the bottom layer.
  // For an echo that exists in only one layer, the vertical extent
  // would therefore be zero.
  // This parameter lets us require that a valid convective echo
  // exist in multiple layers, which is desirable and helps to remove
  // spurious echoes as candidates for convection.
  // The default is 1.0.
  
  void setMinVertExtentForConvectiveKm(double val) {
    _minVertExtentForConvectiveKm = val; 
  }

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
  // Reflectivity threshold for echo tops (dBZ).
  // Echo tops are defined as the max ht with reflectivity at or
  // above this value.
  
  void setDbzForEchoTops(double val) { _dbzForEchoTops = val; }
  
  ////////////////////////////////////////////////////////////////////
  // Set grid details

  void setGrid(size_t nx, size_t ny,
               double dx, double dy,
               double minx, double miny,
               const vector<double> &zKm,
               bool projIsLatLon = false);
  
  ////////////////////////////////////////////////////////////////////
  // compute the echoType
  // Returns 0 on success, -1 on failure

  int computeEchoType(const fl32 *dbz, fl32 dbzMissingVal);

  // get the input fields
  
  const fl32 *getDbz3D() const { return _dbz3D.dat(); }
  const fl32 *getShallowHtGrid() const { return _shallowHtGrid.dat(); }
  const fl32 *getDeepHtGrid() const { return _deepHtGrid.dat(); }
  const fl32 *getDbzColMax() const { return _dbzColMax.dat(); }
  const fl32 *getFractionActive() const { return _fractionActive.dat(); }
  
  ////////////////////////////////////////////////////////////////////
  // get the resulting echoType
  // will be set to the relevant category

  const ui08 *getEchoType3D() const { return _echoType3D.dat(); }
  const ui08 *getEchoType2D() const { return _echoType2D.dat(); }
  const fl32 *getConvectiveDbz() const { return _convDbz.dat(); }

  ////////////////////////////////////////////////////////////////////
  // get derived fields
  
  const fl32 *getTexture3D() const { return _texture3D.dat(); }
  const fl32 *getTexture2D() const { return _texture2D.dat(); }

  const fl32 *getConvectivity3D() const { return _convectivity3D.dat(); }
  const fl32 *getConvectivity2D() const { return _convectivity2D.dat(); }

  const fl32 *getConvTopKm() const { return _convTopKm.dat(); }
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

  // clumping manager
  
  const ClumpingMgr &getClumpingMgr() const { return _clumping; }

  // free up arrays when done, if you want to keep memory usage down

  void freeArrays();

protected:
  
private:

  class StormClump;
  
  // private data
  
  static const fl32 _missingFl32;
  static const ui08 _missingUi08;
  
  bool _debug; // Print debug messages
  bool _verbose; // Print verbose debug messages

  bool _useMultipleThreads;

  double _minValidHtKm;
  double _maxValidHtKm;
  bool _useDbzColMax;
  double _minValidDbz;
  double _baseDbz;

  double _minConvectivityForConvective;
  double _maxConvectivityForStratiform;
  int _minOverlapForClumping;
  
  bool _useDualThresholds;
  double _secondaryConvectivityThreshold;
  double _minFractionAllParts;
  double _minFractionEachPart;
  double _minSizeEachPart;

  double _dbzForEchoTops;

  double _textureRadiusKm;
  double _minValidFractionForTexture;
  double _minValidFractionForFit;
  double _minVolForConvectiveKm3;
  double _minVertExtentForConvectiveKm;

  // specify freezing level, and divergence level, by ht MSL
  // if this is false, grids for fz and div level must be passed in
  
  double _shallowHtKm;
  double _deepHtKm;

  // converting texture to convectivity convectivity
  // these are the limits mapping to 0 and 1
  
  double _textureLimitLow;
  double _textureLimitHigh;

  // grid details

  bool _gridSet;
  
  size_t _nx, _ny;
  double _minx, _miny;
  double _dx, _dy;
  double _dxKm, _dyKm;
  vector<double> _zKm;
  bool _projIsLatLon;

  size_t _nxy, _nxyz;
  size_t _minIz, _maxIz;
  
  int _nxTexture, _nyTexture;

  // kernel computations

  vector<kernel_t> _textureKernelOffsets;

  // clumping the convective regions
  
  ClumpingMgr _clumping;
  vector<StormClump *> _clumps;
  
  // inputs
  
  TaArray<fl32> _dbz3D;
  TaArray<fl32> _shallowHtGrid; // grid for shallow cloud ht threshold
  TaArray<fl32> _deepHtGrid;    // grid for deep cloud ht threshold
  TaArray<fl32> _dbzColMax;
  TaArray<fl32> _fractionActive;
  
  // texture
  
  TaArray<fl32> _texture3D;
  TaArray<fl32> _texture2D;
  TaArray<fl32> _textureColMax;

  // convectivity
  
  TaArray<fl32> _convectivity3D;
  TaArray<fl32> _convectivity2D;

  // echoType
  
  TaArray<ui08> _echoType3D;
  TaArray<ui08> _echoType2D;

  // tops etc

  TaArray<fl32> _convTopKm;
  TaArray<fl32> _stratTopKm;
  TaArray<fl32> _echoTopKm;
  TaArray<fl32> _convDbz;

  // methods
  
  int _computeEchoType2D(const fl32 *dbz, fl32 dbzMissingVal);
  void _allocArrays();
  void _initToMissing();
  void _initToMissing(TaArray<fl32> &array, fl32 missingVal);
  void _initToMissing(TaArray<ui08> &array, ui08 missingVal);
  void _computeDbzColMax();
  void _finalizeEchoType();
  void _computeTextureMultiThreaded();
  void _computeTextureSingleThreaded();
  void _computeConvectivity();
  void _performClumping();
  void _freeClumps();
  void _setEchoType3D();
  void _setEchoType2D();
  void _set2DFields();
  void _computeKernels();
  void _printSettings(ostream &out);

  /////////////////////////////////////////////////////////
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
    
    void setMinValidFractionForTexture(double val)
    {
      _minValidFractionForTexture = val;
    }
    
    void setMinValidFractionForFit(double val)
    {
      _minValidFractionForFit = val;
    }
    
    void setDbz(const fl32 *dbz,
                fl32 missingVal)
    {
      _dbz = dbz;
      _missingVal = missingVal;
    }
    
    void setBaseDbz(double val)
    {
      _baseDbz = val;
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
    double _minValidFractionForTexture;
    double _minValidFractionForFit;
    fl32 _missingVal;
    double _baseDbz;
    const fl32 *_dbz;
    const fl32 *_fractionCovered;
    fl32 *_texture;
    vector<kernel_t> _kernelOffsets;
    
  };

  /////////////////////////////////////////////////////////
  // inner class for clump geometry

  class StormClump
  {  
    
  public:   
    
    // constructor
    
    StormClump(ConvStratFinder *finder,
               const ClumpProps &cprops);
    
    // destructor
    
    virtual ~StormClump();
    
    // compute geom
    
    void computeGeom();

    // Set the echoType based on clump properties
    
    void setEchoType();
    
    // check for stratiform below
    
    bool stratiformBelow();
    
    // get methods

    int getVolumeKm3() const { return _volumeKm3; }
    int getNPtsTotal() const { return _nPtsTotal; }
    int getNPtsShallow() const { return _nPtsShallow; }
    int getNPtsMid() const { return _nPtsMid; }
    int getNPtsDeep() const { return _nPtsDeep; }
    
  private:

    ConvStratFinder *_finder;
    ClumpProps _cprops;
    int _id;
    double _volumeKm3;
    double _vertExtentKm;
    int _nPtsTotal;
    int _nPtsShallow;
    int _nPtsMid;
    int _nPtsDeep;
    
  };

};

#endif
