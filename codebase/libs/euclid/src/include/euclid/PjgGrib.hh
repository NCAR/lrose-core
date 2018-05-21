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
////////////////////////////////////////////////
// Grib Projection
//
////////////////////////////////////////////////
#ifndef _GRIB_PJG_
#define _GRIB_PJG_


#include <euclid/Pjg.hh>

//
// Forward class declarations
//

class PjgGrib : public Pjg {
 public:

  typedef enum {
    DO_XYZ = 0,
    DO_YXZ = 1,
    DO_XZY = 2,
    DO_YZX = 3,
    DO_ZXY = 4,
    DO_ZYX = 5
  } data_ordering_t;

  typedef enum {
    GO_OTHER = 0,
    GO_SN_WE = 1,
    GO_NS_WE = 2,
    GO_SN_EW = 3,
    GO_NS_EW = 4
  } grid_orientation_t;

  /**********************************************************************
   * Constructors
   */
 PjgGrib();
  

  /**********************************************************************
   * Destructor
   */

  virtual ~PjgGrib();

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  void init(const double origin_lat, const double origin_lon,
	    const double latin,
	    const int nx = 1, const int ny = 1, const int nz = 1,
	    const double dx = 1.0, const double dy = 1.0,
	    const double dz = 1.0,
	    const double minx = 0.0, const double miny = 0.0,
	    const double minz = 0.0, 
	    const data_ordering_t data_order = DO_XYZ,
	    const grid_orientation_t grid_orient = GO_SN_WE);
 
 
  //////////////////////
  // Accessor methods //
  //////////////////////

//  /**********************************************************************
//   * getOriginLat() - Retrieve the current value of origin latitude.
//   */
//
//  inline double getOriginLat(void) const {
//    return _originLat;
//  }
//
//  /**********************************************************************
//   * getOriginLat() - Retrieve the current value of origin longitude.
//   */
//
//  inline double getOriginLon(void) const {
//    return _originLon;
//  }

  /***********************************************************************
   * getStandardParallel() - Retrieve the current value of the 
   *                         standard parallel.
   */

  inline double getStandardParallel(void) const {
    return getLat1();
  }

  /***********************************************************************
   * getDataOrdering() - Retrieve the current enumerated value of the 
   *                     data ordering.
   */

  inline data_ordering_t getDataOrdering(void) const {
    return _dataOrder;
  }

  /***********************************************************************
   * get() - Retrieve the current enumerated value of the 
   *                         data ordering.
   */

  inline grid_orientation_t getGridOrientation(void) const {
    return _gridOrientation;
  }

  //////////////////////
  // Printing methods //
  //////////////////////

  /**********************************************************************
   * print() - Prints out information about object.
   */

  void print(void);


protected:

  
private:

//  double _originLat;
//  double _originLon;
//  double _standardParallel;
  data_ordering_t _dataOrder;
  grid_orientation_t _gridOrientation;

};

#endif


