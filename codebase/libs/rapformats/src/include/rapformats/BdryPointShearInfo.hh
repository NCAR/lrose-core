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
// BdryPointShearInfo
//
// C++ wrapper for boundary point shear information.
//
// Nancy Rehak, RAP, NCAR
// PO Box 3000, Boulder, CO, USA
//
// March 2005
//////////////////////////////////////////////////////////////

#ifndef _BdryPointShearInfo_hh
#define _BdryPointShearInfo_hh

#include <cstdio>
#include <iostream>

#include <dataport/port_types.h>
#include <rapformats/bdry_typedefs.h>
#include <toolsa/MemBuf.hh>

using namespace std;

class BdryPointShearInfo
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /************************************************************************
   * Constructors
   */

  BdryPointShearInfo(const int num_pts = 0, const double zbar_cape = 0.0,
		     const double max_shear = 0.0,
		     const double mean_shear = 0.0,
		     const double kmin = 0.0, const double kmax = 0.0);

  /************************************************************************
   * Destructor
   */

  virtual ~BdryPointShearInfo();
  

  //////////////////
  // SPDB methods //
  //////////////////

  /************************************************************************
   * assemble() - Load up the given buffer from the object. Handles byte
   *              swapping.
   *
   * Returns true on success, false on failure.
   */

  bool assemble(MemBuf &mem_buf) const;
  

  /************************************************************************
   * disassemble() - Disassembles a buffer, sets the object values. Handles
   *                 byte swapping.
   *
   * Returns true on success, false on failure.
   */

  bool disassemble(char *&buf, int &len);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /************************************************************************
   * copy(): Copy the given shear information into this object.
   */

  inline void copy(const BdryPointShearInfo &rhs)
  {
    _numPoints = rhs._numPoints;
    _zbarCape = rhs._zbarCape;
    _maxShear = rhs._maxShear;
    _meanShear = rhs._meanShear;
    _kMin = rhs._kMin;
    _kMax = rhs._kMax;
  }
  

  /************************************************************************
   * getNumPoints(): Get the number of horizontal points going with the 
   *                 leading edge point.
   */

  inline int getNumPoints() const
  {
    return _numPoints;
  }
  

  /************************************************************************
   * getZbarCape(): Get the CAPE value in the shear layer.
   */

  inline double getZbarCape() const
  {
    return _zbarCape;
  }
  

  /************************************************************************
   * getMaxShear(): Get the maximum shear in the layer.
   */

  inline double getMaxShear() const
  {
    return _maxShear;
  }
  

  /************************************************************************
   * getMeanShear(): Get the mean shear in the layer.
   */

  inline double getMeanShear() const
  {
    return _meanShear;
  }
  

  /************************************************************************
   * getKMin(): Get the bottom level of the shear layer.
   */

  inline double getKMin() const
  {
    return _kMin;
  }
  

  /************************************************************************
   * getKMax(): Get the top level of the shear layer.
   */

  inline double getKMax() const
  {
    return _kMax;
  }
  

  /************************************************************************
   * setNumPoints(): Set the number of horizontal points going with the
   *                 leading edge point.
   */

  inline void setNumPoints(const int num_points)
  {
    _numPoints = num_points;
  }
  

  /************************************************************************
   * setZbarCape(): Set the CAPE value in the shear layer.
   */

  inline void setZbarCape(const double zbar_cape)
  {
    _zbarCape = zbar_cape;
  }
  

  /************************************************************************
   * setMaxShear(): Set the maximum shear in the layer.
   */

  inline void setMaxShear(const double max_shear)
  {
    _maxShear = max_shear;
  }
  

  /************************************************************************
   * setMeanShear(): Set the mean shear in the layer.
   */

  inline void setMeanShear(const double mean_shear)
  {
    _meanShear = mean_shear;
  }
  

  /************************************************************************
   * setKMin(): Set the bottom level of the shear layer.
   */

  inline void setKMin(const double k_min)
  {
    _kMin = k_min;
  }
  

  /************************************************************************
   * setKMax(): Set the top level of the shear layer.
   */

  inline void setKMax(const double k_max)
  {
    _kMax = k_max;
  }
  

  //////////////////////////
  // Input/Output methods //
  //////////////////////////

  /************************************************************************
   * print(): Print the shear information to the given stream.
   */

  void print(FILE *out) const;
  void print(ostream &out) const;


protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  int _numPoints;          /* number of horizontal points goin with the */
                           /*   leading edge point */
  double _zbarCape;        /* CAPE value in the shear layer */
  double _maxShear;        /* maximum shear in the layer */
  double _meanShear;       /* mean shear in the layer */
  double _kMin;            /* bottom level of shear layer */
  double _kMax;            /* top level of shear layer */
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /************************************************************************
   * _pointShearInfoFromBE(): Convert a boundary point shear info structure
   *                          from big-endian format to native format.
   */

  static void _pointShearInfoFromBE(BDRY_spdb_point_shear_info_t &shear_info);
  

  /************************************************************************
   * _pointShearInfoToBE(): Convert a boundary point shear info structure
   *                        from native format to big-endian format.
   */

  static void _pointShearInfoToBE(BDRY_spdb_point_shear_info_t &shear_info);
  

};


#endif
