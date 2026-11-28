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
///////////////////////////////////////////////////////////////
// Fields.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2003
//
///////////////////////////////////////////////////////////////
//
// Utilities for Fields
//
////////////////////////////////////////////////////////////////

#include "Fields.hh"
#include <dataport/port_types.h>
#include <cstring>

using namespace std;

//////////////////////////////////////
// Is this field currently supported?

bool Fields::isSupported(int fieldId)
  
{
  
  switch (fieldId) {

  case FIELD_DBT:
  case FIELD_DBZ:
  case FIELD_VEL:
  case FIELD_WIDTH:
  case FIELD_DBZC:
  case FIELD_DBT2:
  case FIELD_DBZ2:
  case FIELD_VEL2:
  case FIELD_WIDTH2:
  case FIELD_DBZC2:
    return true;
    break;
  default:
    return false;

  }

}

//////////////////////////////
// return field name from ID

string Fields::getName(int fieldId)
  
{
  
  switch (fieldId) {
  case FIELD_DBT:
    return "DBT";
    break;
  case FIELD_DBZ:
    return "DBZ";
    break;
  case FIELD_VEL:
    return "VEL";
    break;
  case FIELD_WIDTH:
    return "WIDTH";
    break;
  case FIELD_ZDR:
    return "ZDR";
    break;
  case FIELD_DBZC:
    return "DBZC";
    break;
  case FIELD_DBT2:
    return "DBT2";
    break;
  case FIELD_DBZ2:
    return "DBZ2";
    break;
  case FIELD_VEL2:
    return "VEL2";
    break;
  case FIELD_WIDTH2:
    return "WIDTH2";
    break;
  case FIELD_ZDR2:
    return "ZDR2";
    break;
  case FIELD_RRATE2:
    return "RAINRATE2";
    break;
  case FIELD_KDP:
    return "KDP";
    break;
  case FIELD_KDP2:
    return "KDP2";
    break;
  case FIELD_PHIDP:
    return "PHIDP";
    break;
  case FIELD_VELC:
    return "VELC";
    break;
  case FIELD_SQI:
    return "SQI";
    break;
  case FIELD_RHOHV:
    return "RHOHV";
    break;
  case FIELD_RHOHV2:
    return "RHOHV2";
    break;
  case FIELD_DBZC2:
    return "DBZC2";
    break;
  case FIELD_VELC2:
    return "VELC2";
    break;
  case FIELD_SQI2:
    return "SQI2";
    break;
  case FIELD_PHIDP2:
    return "PHIDP2";
    break;
  case FIELD_LDRH:
    return "LDRH";
    break;
  case FIELD_LDRH2:
    return "LDRH2";
    break;
  case FIELD_LDRV:
    return "LDRV";
    break;
  case FIELD_LDRV2:
    return "LDRV2";
    break;
  default:
    return "UNKNOWN";
  }

}

//////////////////////////////
// return byte width from ID

int Fields::getByteWidth(int fieldId)
  
{
  
  switch (fieldId) {

  case FIELD_DBT:
  case FIELD_DBZ:
  case FIELD_VEL:
  case FIELD_WIDTH:
  case FIELD_ZDR:
  case FIELD_DBZC:
  case FIELD_KDP:
  case FIELD_PHIDP:
  case FIELD_VELC:
  case FIELD_SQI:
  case FIELD_RHOHV:
  case FIELD_LDRH:
  case FIELD_LDRV:
    return 1;
    break;

  case FIELD_DBT2:
  case FIELD_DBZ2:
  case FIELD_VEL2:
  case FIELD_WIDTH2:
  case FIELD_ZDR2:
  case FIELD_RRATE2:
  case FIELD_KDP2:
  case FIELD_RHOHV2:
  case FIELD_DBZC2:
  case FIELD_VELC2:
  case FIELD_SQI2:
  case FIELD_PHIDP2:
  case FIELD_LDRH2:
  case FIELD_LDRV2:
    return 2;
    break;

  default:
    return 1;

  }

}

//////////////////////////////////////
// Get name, units, scale, bias
//
// returns 0 on success, -1 on failure

int Fields::getNameUnitsScaleBias(int fieldId, double nyquistVel,
				  string &name, string &units,
				  double &scale, double &bias)
  
{

  name = "unknown";
  units = "unknown";
  scale = 1.0;
  bias = 0.0;

  if (!isSupported(fieldId)) {
    return -1;
  }
  
  switch (fieldId) {
    
  case FIELD_DBT:
  case FIELD_DBT2:
    name = "DBT";
    units = "dBZ";
    scale = 0.5;
    bias = -32.0;
    return 0;
    break;

  case FIELD_DBZ:
  case FIELD_DBZ2:
    name = "DBZ";
    units = "dBZ";
    scale = 0.5;
    bias = -32.0;
    return 0;
    break;
    
  case FIELD_DBZC:
  case FIELD_DBZC2:
    name = "DBC";
    units = "dBZ";
    scale = 0.5;
    bias = -32.0;
    return 0;
    break;

  case FIELD_VEL:
  case FIELD_VEL2:
    name = "VEL";
    units = "m/s";
    scale = nyquistVel / 127.0;
    bias = (-1.0 * nyquistVel) * (128.0 / 127.0);
    return 0;
    break;

  case FIELD_WIDTH:
  case FIELD_WIDTH2:
    name = "SPW";
    units = "m/s";
    scale = nyquistVel / 256.0;
    bias = 0.0;
    return 0;
    break;

  default:
    return -1;

  }

}

//////////////////////////////////////
// convert data field to 8-bit
//
// returns 0 on success, -1 on failure

void Fields::convertTo8Bit(int fieldId, double nyquistVel,
			   const MemBuf &in, MemBuf &out)

{

  // trivial case, width is already 8 bit, copy only

  int width = getByteWidth(fieldId);
  if (width == 1) {
    out = in;
    return;
  }

  // get the 8bit scale and bias

  string name, units;
  double scale, bias;
  getNameUnitsScaleBias(fieldId, nyquistVel, name, units, scale, bias);
  
  // convert

  int nVals = in.getLen() / 2;
  ui16 *iData = (ui16*) in.getPtr();
  ui08 *oData = (ui08*) out.reserve(nVals);
  memset(oData, 0, nVals);
  
  switch (fieldId) {
    
  case FIELD_DBT2:
  case FIELD_DBZ2:
  case FIELD_DBZC2:
    
    for (int i = 0; i < nVals; i++, iData++, oData++) {
      if (*iData != 0) {
	double dbz = (((double) *iData) - 32768.0) / 100.0;
	int oDbz = (int) ((dbz - bias) / scale + 0.5);
	if (oDbz > 255) {
	  oDbz = 255;
	} else if (oDbz < 0) {
	  oDbz = 0;
	}
	*oData = oDbz;
      }
    }
    break;

  case FIELD_VEL2:

    for (int i = 0; i < nVals; i++, iData++, oData++) {
      if (*iData != 0) {
	double vel = (((double) *iData) - 32768.0) / 100.0;
	int oVel = (int) ((vel - bias) / scale + 0.5);
	if (oVel > 255) {
	  oVel = 255;
	} else if (oVel < 0) {
	  oVel = 0;
	}
	*oData = oVel;
      }
    }
    break;

  case FIELD_WIDTH2:

    for (int i = 0; i < nVals; i++, iData++, oData++) {
      if (*iData != 0) {
	double width = (double) *iData / 100.0;
	int oWidth = (int) ((width - bias) / scale + 0.5);
	if (oWidth > 255) {
	  oWidth = 255;
	} else if (oWidth < 0) {
	  oWidth = 0;
	}
	*oData = oWidth;
      }
    }
    break;
    
  default:
    out = in;

  }

}

