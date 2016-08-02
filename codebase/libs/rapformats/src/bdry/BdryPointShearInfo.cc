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
// POBox 3000, Boulder, CO, USA
//
// March 2005
//////////////////////////////////////////////////////////////

#include <cstring>

#include <dataport/bigend.h>
#include <rapformats/BdryPointShearInfo.hh>

using namespace std;

/************************************************************************
 * Constructors
 */

BdryPointShearInfo::BdryPointShearInfo(const int num_pts,
				       const double zbar_cape,
				       const double max_shear,
				       const double mean_shear,
				       const double kmin,
				       const double kmax) :
  _numPoints(num_pts),
  _zbarCape(zbar_cape),
  _maxShear(max_shear),
  _meanShear(mean_shear),
  _kMin(kmin),
  _kMax(kmax)
{
}


/************************************************************************
 * Destructor
 */

BdryPointShearInfo::~BdryPointShearInfo()
{
}


/************************************************************************
 * assemble() - Load up the given buffer from the object. Handles byte
 *              swapping.
 *
 * Returns true on success, false on failure.
 */

bool BdryPointShearInfo::assemble(MemBuf &mem_buf) const
{
  BDRY_spdb_point_shear_info_t spdb_shear_info;
  memset(&spdb_shear_info, 0, sizeof(BDRY_spdb_point_shear_info_t));
	
  spdb_shear_info.num_pts = _numPoints;
  spdb_shear_info.zbar_cape = _zbarCape;
  spdb_shear_info.max_shear = _maxShear;
  spdb_shear_info.mean_shear = _meanShear;
  spdb_shear_info.kmin = _kMin;
  spdb_shear_info.kmax = _kMax;
	
  _pointShearInfoToBE(spdb_shear_info);
	
  mem_buf.add((void *)&spdb_shear_info,
	      sizeof(BDRY_spdb_point_shear_info_t));
      
  return true;
}


/************************************************************************
 * disassemble() - Disassembles a buffer, sets the object values. Handles
 *                 byte swapping.
 *
 * Returns true on success, false on failure.
 */

bool BdryPointShearInfo::disassemble(char *&buf, int &len)
{
  static const string method_name = "BdryPointShearInfo::disassemble()";
  
  int info_size = sizeof(BDRY_spdb_point_shear_info_t);
  if (info_size > len)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Incoming buffer too small for product" << endl;
    
    return false;
  }

  BDRY_spdb_point_shear_info_t *spdb_shear_info =
    (BDRY_spdb_point_shear_info_t *)buf;
	
  _pointShearInfoFromBE(*spdb_shear_info);
	
  _numPoints = spdb_shear_info->num_pts;
  _zbarCape = spdb_shear_info->zbar_cape;
  _maxShear = spdb_shear_info->max_shear;
  _meanShear = spdb_shear_info->mean_shear;
  _kMin = spdb_shear_info->kmin;
  _kMax = spdb_shear_info->kmax;
	
  buf += info_size;
  len -= info_size;
  
  return true;
}


/************************************************************************
 * print(): Print the boundary point to the given stream.
 */

void BdryPointShearInfo::print(FILE *stream) const
{
  fprintf(stream, "   num points = %d\n", _numPoints);
  fprintf(stream, "   zbar cape = %f\n", _zbarCape);
  fprintf(stream, "   max shear = %f\n", _maxShear);
  fprintf(stream, "   mean shear = %f\n", _meanShear);
  fprintf(stream, "   k min = %f\n", _kMin);
  fprintf(stream, "   k max = %f\n", _kMax);
}

void BdryPointShearInfo::print(ostream &stream) const
{
  stream << "   num points = " << _numPoints << endl;
  stream << "   zbar cape = " << _zbarCape << endl;
  stream << "   max shear = " << _maxShear << endl;
  stream << "   mean shear = " << _meanShear << endl;
  stream << "   k min = " << _kMin << endl;
  stream << "   k max = " << _kMax << endl;
}

/************************************************************************
 * STATIC ROUTINES
 ************************************************************************/

/************************************************************************
 * _pointShearInfoFromBE(): Convert a boundary point shear info structure
 *                          from big-endian format to native format.
 */

void BdryPointShearInfo::_pointShearInfoFromBE(BDRY_spdb_point_shear_info_t &shear_info)
{
  /*
   * This structure contains only fl32 values, so convert as an
   * array.
   */

  BE_to_array_32(&shear_info, sizeof(BDRY_spdb_point_shear_info_t));
  
  return;
}


/************************************************************************
 * _pointShearInfoToBE(): Convert a boundary point shear info structure
 *                        from native format to big-endian format.
 */

void BdryPointShearInfo::_pointShearInfoToBE(BDRY_spdb_point_shear_info_t &shear_info)
{
  /*
   * This structure contains only fl32 values, so convert as an
   * array.
   */

  BE_from_array_32(&shear_info, sizeof(BDRY_spdb_point_shear_info_t));
  
  return;
}
