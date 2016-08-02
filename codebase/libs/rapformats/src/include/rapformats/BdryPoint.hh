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
// BdryPoint
//
// C++ wrapper for boundary point data.
//
// Nancy Rehak, RAP, NCAR
// PO Box 3000, Boulder, CO, USA
//
// March 2005
//////////////////////////////////////////////////////////////

#ifndef _BdryPoint_hh
#define _BdryPoint_hh

#include <cstdio>
#include <iostream>

#include <dataport/port_types.h>
#include <rapformats/bdry_typedefs.h>
#include <rapformats/BdryPointShearInfo.hh>
#include <toolsa/MemBuf.hh>

using namespace std;

class BdryPoint
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /************************************************************************
   * Constructors
   */

  BdryPoint(const double lat = 0.0, const double lon = 0.0,
	    const double u_comp = 0.0, const double v_comp = 0.0,
	    const double steering_flow = 0.0);

  BdryPoint(const BdryPoint &rhs);
  

  /************************************************************************
   * Destructor
   */

  virtual ~BdryPoint();
  

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
   * getLat(): Get the latitude of the point.
   */

  inline double getLat() const
  {
    return _lat;
  }
  

  /************************************************************************
   * getLon(): Get the longitude of the point.
   */

  inline double getLon() const
  {
    return _lon;
  }
  

  /************************************************************************
   * getUComp(): Get the U component of the point.
   */

  inline double getUComp() const
  {
    return _uComp;
  }
  

  /************************************************************************
   * getVComp(): Get the V component of the point.
   */

  inline double getVComp() const
  {
    return _vComp;
  }
  

  /************************************************************************
   * getSteeringFlow(): Get the boundary relative steering flow value
   *                    of the point.
   */

  inline double getSteeringFlow() const
  {
    return _steeringFlow;
  }
  

  /************************************************************************
   * hasShearInfo(): Get a flag indicating whether this point has the
   *                 optional shear information.
   */

  inline bool hasShearInfo() const
  {
    return !(_shearInfo == 0);
  }
  

  /************************************************************************
   * getShearInfo(): Get the shear information for the point.
   */

  inline BdryPointShearInfo getShearInfo() const
  {
    if (_shearInfo)
      return *_shearInfo;
    
    return BdryPointShearInfo();
  }
  

  /************************************************************************
   * setLocation(): Set the location of the point.
   */

  inline void setLocation(const double lat, const double lon)
  {
    _lat = lat;
    _lon = lon;
  }
  

  /************************************************************************
   * setLat(): Set the latitude of the point.
   */

  inline void setLat(const double lat)
  {
    _lat = lat;
  }
  

  /************************************************************************
   * setLon(): Set the longitude of the point.
   */

  inline void setLon(const double lon)
  {
    _lon = lon;
  }
  

  /************************************************************************
   * setUComp(): Set the U component of the point.
   */

  inline void setUComp(const double u_comp)
  {
    _uComp = u_comp;
  }
  

  /************************************************************************
   * setVComp(): Set the V component of the point.
   */

  inline void setVComp(const double v_comp)
  {
    _vComp = v_comp;
  }
  

  /************************************************************************
   * setShearInfo(): Set the shear information for the point.
   */

  inline void setShearInfo(const BdryPointShearInfo &shear_info)
  {
    if (_shearInfo)
      _shearInfo->copy(shear_info);
    else
      _shearInfo = new BdryPointShearInfo(shear_info);
  }
  

  /************************************************************************
   * setSteeringFlow(): Set the boundary relative steering flow value
   *                    of the point.
   */

  inline void setSteeringFlow(const double steering_flow)
  {
    _steeringFlow = steering_flow;
  }
  

  //////////////////////////
  // Input/Output methods //
  //////////////////////////

  /************************************************************************
   * print(): Print the boundary point to the given stream.
   */

  void print(FILE *out) const;
  void print(ostream &out) const;


  ///////////////
  // Operators //
  ///////////////

  /************************************************************************
   * operator=()
   */

  BdryPoint &operator=(const BdryPoint &rhs);


protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  double _lat;                   /* latitude in degrees */
  double _lon;                   /* longitude in degrees */
  
  double _uComp;                 /* horizontal motion dir vector component */
                                 /*   in m/s */
  double _vComp;                 /* vertical motion dir vector component */
                                 /*   in m/s */
  
  double _steeringFlow;          /* bdry rel steering flow value for the */
                                 /*   initiation zone */
  
  BdryPointShearInfo *_shearInfo; /* optional boundary point shear info */
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /************************************************************************
   * _pointFromBE(): Convert a boundary point structure from big-endian
   *                 format to native format.
   */

  static void _pointFromBE(BDRY_spdb_point_t &point);
  

  /************************************************************************
   * _pointToBE(): Convert a boundary point structure from native format
   *               to big-endian format.
   */

  static void _pointToBE(BDRY_spdb_point_t &point);
  

};


#endif
