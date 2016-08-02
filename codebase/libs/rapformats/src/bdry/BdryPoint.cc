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
// POBox 3000, Boulder, CO, USA
//
// March 2005
//////////////////////////////////////////////////////////////

#include <cstring>

#include <dataport/bigend.h>
#include <rapformats/BdryPoint.hh>

using namespace std;

/************************************************************************
 * Constructors
 */

BdryPoint::BdryPoint(const double lat, const double lon,
		     const double u_comp, const double v_comp,
		     const double steering_flow) :
  _lat(lat),
  _lon(lon),
  _uComp(u_comp),
  _vComp(v_comp),
  _steeringFlow(steering_flow),
  _shearInfo(0)
{
}


BdryPoint::BdryPoint(const BdryPoint &rhs) :
  _lat(rhs._lat),
  _lon(rhs._lon),
  _uComp(rhs._uComp),
  _vComp(rhs._vComp),
  _steeringFlow(rhs._steeringFlow),
  _shearInfo(0)
{
  if (rhs._shearInfo)
    _shearInfo = new BdryPointShearInfo(*rhs._shearInfo);
}


/************************************************************************
 * Destructor
 */

BdryPoint::~BdryPoint()
{
}


/************************************************************************
 * operator=()
 */

BdryPoint &BdryPoint::operator=(const BdryPoint &rhs)
{
  if (&rhs == this)
    return *this;
  
  _lat = rhs._lat;
  _lon = rhs._lon;
  _uComp = rhs._uComp;
  _vComp = rhs._vComp;
  _steeringFlow = rhs._steeringFlow;
  
  delete _shearInfo;
  _shearInfo = new BdryPointShearInfo(*rhs._shearInfo);
  
  return *this;
}


/************************************************************************
 * assemble() - Load up the given buffer from the object. Handles byte
 *              swapping.
 *
 * Returns true on success, false on failure.
 */

bool BdryPoint::assemble(MemBuf &mem_buf) const
{
  // Get the default point information

  BDRY_spdb_point_t spdb_point;
  memset(&spdb_point, 0, sizeof(BDRY_spdb_point_t));
      
  spdb_point.lat     = _lat;
  spdb_point.lon     = _lon;
  spdb_point.u_comp  = _uComp;
  spdb_point.v_comp  = _vComp;
  spdb_point.value   = _steeringFlow;
      
  spdb_point.info_mask = 0;
  if (hasShearInfo())
    spdb_point.info_mask |= BDRY_POINT_INFO_SHEAR;
      
  _pointToBE(spdb_point);
      
  mem_buf.add((void *)&spdb_point, sizeof(BDRY_spdb_point_t));
      
  // Add the point shear information

  if (hasShearInfo())
    _shearInfo->assemble(mem_buf);
      
  return true;
}


/************************************************************************
 * disassemble() - Disassembles a buffer, sets the object values. Handles
 *                 byte swapping.
 *
 * Returns true on success, false on failure.
 */

bool BdryPoint::disassemble(char *&buf, int &len)
{
  static const string method_name = "BdryPoint::disassemble()";
  
  int point_size = sizeof(BDRY_spdb_point_t);
  if (point_size > len)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Incoming buffer too small for product" << endl;
    
    return false;
  }

  BDRY_spdb_point_t *spdb_point = (BDRY_spdb_point_t *)buf;
  _pointFromBE(*spdb_point);
  
  _lat = spdb_point->lat;
  _lon = spdb_point->lon;
  _uComp = spdb_point->u_comp;
  _vComp = spdb_point->v_comp;
  _steeringFlow = spdb_point->value;
      
  buf += point_size;
  len -= point_size;
  
  if ((spdb_point->info_mask & BDRY_POINT_INFO_SHEAR) > 0)
  {
    _shearInfo = new BdryPointShearInfo();
    if (!_shearInfo->disassemble(buf, len))
      return false;
  }
  
  return true;
}


/************************************************************************
 * print(): Print the boundary point to the given stream.
 */

void BdryPoint::print(FILE *stream) const
{
  fprintf(stream, "   lat = %f\n", _lat);
  fprintf(stream, "   lon = %f\n", _lon);
  fprintf(stream, "   u_comp = %f\n", _uComp);
  fprintf(stream, "   v_comp = %f\n", _vComp);
  fprintf(stream, "   steering_flow = %f\n", _steeringFlow);

  if (_shearInfo)
    _shearInfo->print(stream);
}

void BdryPoint::print(ostream &stream) const
{
  stream << "   lat = " << _lat << endl;
  stream << "   lon = " << _lon << endl;
  stream << "   u_comp = " << _uComp << endl;
  stream << "   v_comp = " << _vComp << endl;
  stream << "   steering_flow = " << _steeringFlow << endl;

  if (_shearInfo)
    _shearInfo->print(stream);
}

/************************************************************************
 * STATIC ROUTINES
 ************************************************************************/

/************************************************************************
 * _pointFromBE(): Convert a boundary point structure from big-endian
 *                 format to native format.
 */

void BdryPoint::_pointFromBE(BDRY_spdb_point_t &point)
{
  /*
   * This structure contains only fl32 values, so convert as an
   * array.
   */

  BE_to_array_32(&point, sizeof(BDRY_spdb_point_t));
  
  return;
}


/************************************************************************
 * _pointToBE(): Convert a boundary point structure from native format
 *               to big-endian format.
 */

void BdryPoint::_pointToBE(BDRY_spdb_point_t &point)
{
  /*
   * This structure contains only fl32 values, so convert as an
   * array.
   */

  BE_from_array_32(&point, sizeof(BDRY_spdb_point_t));
  
  return;
}
