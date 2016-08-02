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
// GribMgr.hh
//
// Carl Drews, Mike Dixon
// RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2004
//
// Copied from Grib2Mdv.
// Wafs-specific functionality added in from WafsGribMgr.
//
///////////////////////////////////////////////////////////////

#ifndef _GRIB_MGR
#define _GRIB_MGR

#include <dataport/port_types.h>
#include <euclid/Pjg.hh>
#include <grib/IdSec.hh>
#include <grib/PDS.hh>
#include <grib/GDS.hh>
#include <grib/BMS.hh>
#include <grib/BDS.hh>
#include <grib/EquidistantCylind.hh>
using namespace std;

#define GRIB_INDICATOR "GRIB"

// Forward class declarations

/**
 *  \class GribMgr
 *  \author Carl Drews
 *  This class is the base class for various types of grib file
 *  converters (ruc, avn, eta).
 *  GribMgr and its derivatives implement the Strategy design pattern.
 */

class GribMgr {

public:

  GribMgr();
  virtual ~GribMgr();

  /// Scans the GRIB record to determine what is in it.
  int inventoryRecord( ui08 *gribPtr );

  /// Extracts the packed information from a single GRIB record.
  int unpackRecord( ui08 *gribPtr, int nx, int ny );

  /// Retrieves the GDS as an Equidistant Cylindrical projection.
  /// This routine replaces the existing _gds object.
  /// \param sectionPtr where to start unpacking
  /// \return error code or 0 if success
  int loadEquidistantCylindrical(ui08 *sectionPtr);

  /// Some GRIB files have a header before the first 'GRIB' indicator.
  /// This header is illegal according to the formal specification in
  /// http://www.nco.ncep.noaa.gov/pmb/docs/on388/
  /// But we still have to support those files.
  /// This routine advances the file pointer until the first 'GRIB'.
  /// \param fp pointer to a newly opened grib file.  Possibly advanced upon return.
  void findFirstRecord(FILE *fp);

  inline void reset(void){ _forecastTime = -1; }

  inline int getForecastTime() const { return( _forecastTime ); }
  inline time_t getGenerateTime() const { return( _generateTime ); }

  inline int getParameterId() const { return( _pds.getParameterId() ); }
  inline int getLevelId() const { return( _pds.getLevelId() ); }

  /// Sometimes the level is based on the type and bottom-top boundaries.
  int getLevel() const { return( _pds.getLevelVal() ); }

  inline string getLongName() const { return( _pds.getLongName() ); }
  inline string getName() const { return( _pds.getName() ); }
  inline string getUnits() const { return( _pds.getUnits() ); }

  inline bool isOneLevelVal() const { return( _pds.isOneLevelVal() ); }
  inline int getLevelVal() const { return( _pds.getLevelVal() ); }
  inline int getLevelValTop() const { return( _pds.getLevelValTop() ); }
  inline int getLevelValBottom() const { return( _pds.getLevelValBottom() ); }

  inline const fl32 *getData() const { return( _bds.getConstData() ); }
  inline int getNumValues() const { return( _bds.getNumValues() ); }
  inline const Pjg *getProjection() const { return &(_gds.getProjection()); }

  void setProjection(Pjg &newProjection)
  { _gds.setProjection(newProjection); }

  static inline fl32 getMissingVal() { return( BDS::MISSING_DATA ); }

  inline double getFirstLat() const {return (_gds.getFirstLat()); }
  inline double getFirstLon() const {return (_gds.getFirstLon()); }
  inline double getOriginLat() const {return (_gds.getOriginLat()); }
  inline double getOriginLon() const {return (_gds.getOriginLon()); }
  inline double getLatin1() const { return (_gds.getLatin1()); }
  inline double getLatin2() const {return (_gds.getLatin2()); }

  /// Retrieve the grib record's orientation
  /// (South-to-North, West-to-East, etc.)
  GDS::grid_orientation_t getGridOrientation() const {
    return _gds.getGridOrientation();
  }

  /// Swap the GDS orientation from North-South to South-North
  void swapGridOrientationNS_2_SN();

  void printPDS(FILE *stream) const { _pds.print(stream); }

  /// Some GRIB formats have quasi-regular grids where the number of rows is not fixed,
  /// but decreases toward the poles.  These irregular grids are a problem for MDV and others.

  /// When we encounter these grids we re-map them to a straight rectangle.
  void mapQuasiToRegular();
  
protected:

  IdSec _id;
  PDS _pds;
  EquidistantCylind _gds;
  BMS _bms;
  BDS _bds;

  //
  // These will only be used if the file type is ISOBARIC
  //
  double _pressureDelta;
  double _pressureLowest;

  int _forecastTime;
  time_t _generateTime;
  
   
  /// Regrid data from one equally-spaced grid to a different
  /// equally-spaced grid.
  /// \param y values of a function defined on an equally-spaced domain, y[0], ..., y[n]
  /// \param n number of input y values
  /// \param v output values regridded onto new equally-spaced grid over same domain
  /// \param m number of output v values
  /// \param c m precomputed interpolation coefficients:
  ///                                 for (j=0; j<m; j++)
  ///                                     c[j] = (double) (m - j -1) / (m - 1);
  void _linear(fl32 y[], int n, fl32 v[], int m, double c[]);

  /// Convert an irregular grid to a regular grid.
  /// \param nrows number of rows in input
  /// \param ix row i starts at idat[ix[i]], and ix[nrows] is 1 after last elem of idat
  /// \param idat input quasi-regular data
  /// \param ni constant length of each output row
  /// \param nj number of output rows
  /// \param odat where to put ni*nj outputs (already allocated)
  void _qlin(int nrows, int ix[], fl32 idat[], int ni, int nj, fl32 odat[]);

};

#endif
