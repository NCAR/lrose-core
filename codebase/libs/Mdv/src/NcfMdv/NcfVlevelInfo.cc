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
// NcfVlevelInfo.hh
//
// Vertical level information object for gridded dataset
//
// Sue Dettling, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2008
//
///////////////////////////////////////////////////////////////
//
//  NcfVlevelInfo object will bridge the Mdv vlevel info with the
//  netCDF dimensions and vertical coordinate variables.
//
///////////////////////////////////////////////////////////////////////

#include <cstring>
#include <Mdv/NcfVlevelInfo.hh>
#include <cstring>
#include <Mdv/NcfMdv.hh>
#include <cstring>
#include <toolsa/TaStr.hh>
#include <toolsa/mem.h>

//
// Extract NetCDF vlevel meta data from the Mdv vlevel_header_t including
// standard_name, long_name, units, and positive.
//
NcfVlevelInfo::NcfVlevelInfo (Mdvx::vlevel_header_t &vlevHdr, int vlevelType,
                        int nz) :
        _vlevHdr(vlevHdr)
{

  _nz = nz;

  MEM_zero(_zData);
  memcpy(_zData,  _vlevHdr.level, _nz* sizeof(fl32));

  _vlevelType = vlevelType;
  
  switch( _vlevelType)
    {
    case Mdvx::VERT_TYPE_SURFACE:

      _standardName = "";

      _longName = "surface";

      _units = NcfMdv::level;

      _positive = NcfMdv::up;

      break;

    case Mdvx::VERT_TYPE_SIGMA_P:
      
      _standardName = "atmosphere_sigma_coordinate";

      _units = NcfMdv::sigma_level;

      _longName = "sigma p levels";

      _positive = NcfMdv::up;
      
      break;	

    case  Mdvx::VERT_TYPE_PRESSURE:
      
      _standardName = "air_pressure";

      _units = "mb";

      _longName = "pressure levels";

      //
      // This is an optional attribute since pressure levels
      // can be identified by pressure units.
      //
      _positive = NcfMdv::down;
 
      break;	     

    case  Mdvx::VERT_TYPE_Z:
      
      _standardName = "altitude";

      _units = "km";

      _longName = "constant altitude levels";

      _positive = NcfMdv::up;

      break;	         
      
    case  Mdvx::VERT_TYPE_SIGMA_Z:
      
      _standardName = "atmosphere_sigma_coordinate";

      _units = NcfMdv::level;

      _longName = "sigma z levels";

      _positive = NcfMdv::up;
      
      break;	
   
    case  Mdvx::VERT_TYPE_ETA:
      
      _standardName = "";

      _units = NcfMdv::level;

      _longName = "model eta levels";

      _positive = NcfMdv::up;

      break;
      
    case  Mdvx::VERT_TYPE_THETA:
      
      _standardName = "";

      _units = "degree_Kelvin";

      _longName = "isentropic surface";

      _positive = NcfMdv::up;

      break;	 
         
    case  Mdvx::VERT_TYPE_ELEV:
      
      _standardName = NcfMdv::degrees;

      _units = "degree";

      _longName = "elevation angles";

      _positive = NcfMdv::up;

      break;	 

    case Mdvx::VERT_FLIGHT_LEVEL:

      _standardName = "";

      _units = "100 feet";

      _longName = "Flight levels in 100s of feet";

      _positive = NcfMdv::up;

      break;	 

    default:
      
      _standardName = "";
      
      _units = NcfMdv::level;
      
      _longName = "vertical level type unknown";
      
      //
      //  this is a best guess, maybe should be a field_param
      //
      _positive = NcfMdv::up;
      
      break;
    }
}

NcfVlevelInfo::~NcfVlevelInfo()
{
 

}

//////////////////////////////////////////////////////////
// Operator equals()
// Two VelvelInfo objects are equal if the number of levels,
// vertical types are equal and all the vlevel data agrees.
//

bool NcfVlevelInfo::operator==(const NcfVlevelInfo &other)

{

  //
  // check nz  and vlevel type
  //
  if (_nz != other._nz || _vlevelType != other._vlevelType) {
    return false;
  }

  //
  // Check the vlevel data 
  //
  for (int i = 0; i < _nz; i++) {
    if (fabs( _zData[i] - other._zData[i]) > VLEVEL_EPSILON) {
      return false;
    }
  }
  
  return true;

}


////////////////////////////////////////////////////////////
// add dimension for this vlevel object
// returns 0 on success, -1 on failure

int NcfVlevelInfo::addDim(int vlevelNum, Nc3File *ncFile, string &errStr)

{

  char zDimName[4];
  sprintf(zDimName, "z%d", vlevelNum);

  _zDim = ncFile->add_dim(zDimName, _nz);
  if (_zDim == NULL) {
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
// add vert level variable

int NcfVlevelInfo::addVlevelVar(int vlevelNum, Nc3File *ncFile, string &errStr)

{

  // Add variable to NcFile
  
  char zVarName[32];
  sprintf(zVarName, "z%d", vlevelNum);
  
  if ((_zVar = ncFile->add_var(zVarName, nc3Float, _zDim)) == NULL) {
    TaStr::AddStr(errStr, "Mdv2NcfTrans::NcfVlevelInfo::addVlevelVar");
    TaStr::AddStr(errStr, "  Cannot add zVar");
    return -1;
  }
  
  // Add Attributes

  int iret = 0;

  if (_standardName.size() > 0) {
    iret |= !_zVar->add_att(NcfMdv::standard_name, _standardName.c_str());
  }

  // for vert section ??
  // iret |= !_zVar->add_att("standard_name", "z");
  
  iret |= !_zVar->add_att(NcfMdv::long_name, _longName.c_str());

  // for vert section ??
  // iret |= !_zVar->add_att("long_name",
  //                         "distance from trajectory in vertical dimension");

  if (_units != string("level")){
    iret |= !_zVar->add_att(NcfMdv::units, _units.c_str());
  }

  if (_positive.size() > 0) {
    iret |= !_zVar->add_att(NcfMdv::positive, _positive.c_str());
  }

  iret |= !_zVar->add_att(NcfMdv::axis, "Z");

  return (iret? -1 : 0);
  
}

///////////////////////////////////////////////////////////
// Write the vlevel data
//
// Returns 0 on success, -1 on error

int NcfVlevelInfo::writeDataToFile(Nc3File *ncFile, string &errStr)
  
{
  
  if (_zVar != NULL) {
    if (!_zVar->put( _zData, _nz)) {
      TaStr::AddStr(errStr, "ERROR - NcfVlevelInfo::writeVlevelDataToFile");
      TaStr::AddStr(errStr, "  Cannot put vlevel data");
      return -1;
    }
  }

  return 0;

}
