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
// ConvStrat.hh
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

#ifndef ConvStrat_HH
#define ConvStrat_HH

#include <string>
#include <vector>
#include <toolsa/TaArray.hh>
#include <toolsa/TaThread.hh>
#include <dataport/port_types.h>
using namespace std;

////////////////////////
// This class

class ConvStrat {
  
public:

  typedef enum {
    CATEGORY_MISSING = 0,
    CATEGORY_STRATIFORM = 1,
    CATEGORY_CONVECTIVE = 2,
    CATEGORY_UNKNOWN
  } category_t;
  
  // constructor
  
  ConvStrat();
  
  // destructor
  
  ~ConvStrat();

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
  // Radius of convective influence (km).  Given definite convection
  // at a point (see above), we set all points within this radius to
  // be convective.

  void setConvectiveRadiusKm(double val) { _convectiveRadiusKm = val; }

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
  // Set grid details

  void setGrid(int nx, int ny, 
               double dx, double dy,
               double minx, double miny,
               const vector<double> &zKm,
               bool projIsLatLon = false)
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
  }

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
  
  ////////////////////////////////////////////////////////////////////
  // get the resulting partition
  // will be set to the relevant category

  const ui08 *getPartition() const { return _partition.buf(); }
  const fl32 *getConvectiveDbz() const { return _convDbz.buf(); }
  const fl32 *getStratiformDbz() const { return _stratDbz.buf(); }

  ////////////////////////////////////////////////////////////////////
  // get intermediate fields for debugging

  const fl32 *getMeanTexture() const { return _meanTexture.buf(); }
  const fl32 *getFractionActive() const { return _fractionActive.buf(); }
  const fl32 *getColMaxDbz() const { return _colMaxDbz.buf(); }
  const ui08 *getConvFromColMax() const { return _convFromColMax.buf(); }
  const ui08 *getConvFromTexture() const { return _convFromTexture.buf(); }

  // get missing value for float arrays

  fl32 getMissingVal() const { return _missing; }

  // get grid details

  int getGridNx() const { return _nx; }
  int getGridNy() const { return _ny; }

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

  static const fl32 _missing;

  bool _debug;   /**< Print debug messages */
  bool _verbose; /**< Print verbose debug messages */

  double _minValidHtKm;
  double _maxValidHtKm;
  double _minValidDbz;
  double _dbzForDefiniteConvection;
  double _convectiveRadiusKm;
  double _textureRadiusKm;
  double _minValidFractionForTexture;
  double _minTextureForConvection;

  vector<ssize_t> _textureKernelOffsets;
  vector<ssize_t> _convKernelOffsets;

  int _nx, _ny;
  double _minx, _miny;
  double _dx, _dy;
  vector<double> _zKm;
  bool _projIsLatLon;

  int _nxy, _nxyz;
  int _minIz, _maxIz;

  int _nxTexture, _nyTexture;
  int _nxConv, _nyConv;

  TaArray<ui08> _partition;
  TaArray<fl32> _convDbz;
  TaArray<fl32> _stratDbz;

  TaArray<fl32> _volDbz;
  TaArray<fl32> _volTexture;
  TaArray<fl32> _sumTexture;
  TaArray<fl32> _nTexture;
  TaArray<fl32> _meanTexture;
  TaArray<fl32> _fractionActive;
  TaArray<fl32> _colMaxDbz;
  TaArray<ui08> _convFromColMax;
  TaArray<ui08> _convFromTexture;

  void _allocArrays();
  void _computeColMax();
  void _setPartition();
  void _computeTexture();
  void _computeKernels();
  void _printSettings(ostream &out);

  // inner class for starting timers in a separate thread

  class ComputeTexture : public TaThread
  {  
    
  public:   
    
    // constructor saves _sd3c pointer
    
    ComputeTexture(int iz);
    
    // destructor
    
    virtual ~ComputeTexture();
    
    // set parameters
    
    void setGridSize(int nx, int ny)
    {
      _nx = nx;
      _ny = ny;
    }
    
    void setKernelSize(int nx, int ny)
    {
      _nxTexture = nx;
      _nyTexture = ny;
    }
    
    void setMinValidFraction(double val)
    {
      _minValidFraction = val;
    }
    
    void setDbz(const fl32 *dbz, fl32 missingVal)
    {
      _dbz = dbz;
      _missingVal = missingVal;
    }
    
    void setFractionCovered(const fl32 *frac)
    {
      _fractionCovered = frac;
    }
    
    void setKernelOffsets(const vector<ssize_t> &offsets)
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
    
    int _iz;
    int _nx, _ny;
    int _nxTexture, _nyTexture;
    double _minValidFraction;
    fl32 _missingVal;
    const fl32 *_dbz;
    const fl32 *_fractionCovered;
    fl32 *_texture;
    vector<ssize_t> _kernelOffsets;
    
  };

};

#endif
