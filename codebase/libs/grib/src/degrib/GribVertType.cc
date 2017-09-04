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
/////////////////////////////////////////////
// GribVertType - Class providing utilities for using the GRIB
//                vertical level type
//
////////////////////////////////////////////

#include <iostream>

#include <grib/GribVertType.hh>

using namespace std;

GribVertType::GribVertType(const vert_type_t vert_type,
			   const int level_value,
			   const int level_value_bottom)
{
  set(vert_type, level_value, level_value_bottom);
}

GribVertType::~GribVertType() 
{
  // Do nothing
}

void GribVertType::set(const vert_type_t vert_type,
		       const int level_value,
		       const int level_value_bottom)
{
  _vertType = vert_type;
  
  switch (_vertType)
  {
  case SURFACE : 
  case CLOUD_BASE : 
  case CLOUD_TOPS : 
  case ZERO_ISOTHERM : 
  case ADIABATIC_CONDENSATION :
  case MAX_WIND : 
  case TROPOPAUSE : 
  case NOMINAL_ATM_TOP : 
  case MEAN_SEA_LEVEL : 
  case POTENTIAL_VORTICITY : 
  case ENTIRE_ATMOSPHERE :
  case UNKNOWN_10 : 
  case FREEZING_LEVEL :
  case GCBL :
  case GCTL :
  case UNKNOWN_211 :
  case UNKNOWN_212 :
  case UNKNOWN_214 :
  case UNKNOWN_213 :
  case UNKNOWN_222 :
  case UNKNOWN_223 :
  case UNKNOWN_224 :
  case UNKNOWN_232 :
  case UNKNOWN_233 :
  case UNKNOWN_234 :
  case UNKNOWN_244 :
  case LLTW :
  case MAX_EQ_THETA_PRESSURE : 
  case EQUILIBRIUM_LEVEL_HEIGHT : 
  case SCGL : 
  case SCTL : 
  case DCBL : 
  case DCTL : 
    _singleLevelValue = true;
    _levelValue = 0;
    _levelValueBottom = 0;
    break;

  case ISOBARIC : 
  case ALTITUDE_ABOVE_MSL : 
  case HEIGHT_ABOVE_GROUND : 
  case SIGMA : 
  case HYBRID : 
  case DEPTH_BELOW_LAND_SURFACE : 
  case CONV_CLD_TOP : 
  case UNKNOWN_242 :
    _singleLevelValue = true;
    _levelValue = level_value;
    _levelValueBottom = 0;
    break;

  case BETWEEN_ISOBARIC : 
  case BETWEEN_HT_ABOVE_GROUND : 
  case BETWEEN_SIGMA : 
  case BETWEEN_DEPTH : 
  case BETWEEN_PRS_DIFF : 
    _singleLevelValue = false;
    _levelValue = level_value;
    _levelValueBottom = level_value_bottom;
    break;

  default:
    cerr << "unknown Level Indicator = " << _vertType << endl;
    _vertType = SURFACE;
    _singleLevelValue = true;
    _levelValue = 0;
    _levelValueBottom = 0;
    break;
  }

}

void GribVertType::set(const ui08 octet9, const ui08 octet10,
		       const ui08 octet11, const int octet10_11) 
{
  _vertType = (GribVertType::vert_type_t) octet9;

  switch (_vertType)
  {
  case GribVertType::SURFACE : 
  case GribVertType::CLOUD_BASE : 
  case GribVertType::CLOUD_TOPS : 
  case GribVertType::ZERO_ISOTHERM : 
  case GribVertType::ADIABATIC_CONDENSATION :
  case GribVertType::MAX_WIND : 
  case GribVertType::TROPOPAUSE : 
  case GribVertType::NOMINAL_ATM_TOP : 
  case GribVertType::MEAN_SEA_LEVEL : 
  case GribVertType::POTENTIAL_VORTICITY : 
  case GribVertType::ENTIRE_ATMOSPHERE :
  case GribVertType::UNKNOWN_10 : 
  case GribVertType::FREEZING_LEVEL :
  case GribVertType::GCBL :
  case GribVertType::GCTL :
  case GribVertType::UNKNOWN_211 :
  case GribVertType::UNKNOWN_212 :
  case GribVertType::UNKNOWN_214 :
  case GribVertType::UNKNOWN_213 :
  case GribVertType::UNKNOWN_222 :
  case GribVertType::UNKNOWN_223 :
  case GribVertType::UNKNOWN_224 :
  case GribVertType::UNKNOWN_232 :
  case GribVertType::UNKNOWN_233 :
  case GribVertType::UNKNOWN_234 :
  case GribVertType::UNKNOWN_244 :
  case GribVertType::LLTW :
  case GribVertType::MAX_EQ_THETA_PRESSURE : 
  case GribVertType::EQUILIBRIUM_LEVEL_HEIGHT : 
  case GribVertType::SCGL : 
  case GribVertType::SCTL : 
  case GribVertType::DCBL : 
  case GribVertType::DCTL : 
    _singleLevelValue = true;
    _levelValue = 0;
    _levelValueBottom = 0;
    break;

  case GribVertType::ISOBARIC : 
  case GribVertType::ALTITUDE_ABOVE_MSL : 
  case GribVertType::HEIGHT_ABOVE_GROUND : 
  case GribVertType::SIGMA : 
  case GribVertType::HYBRID : 
  case GribVertType::DEPTH_BELOW_LAND_SURFACE : 
  case GribVertType::CONV_CLD_TOP : 
  case GribVertType::UNKNOWN_242 :
    _singleLevelValue = true;
    _levelValue = octet10_11;
    _levelValueBottom = 0;
    break;

  case GribVertType::BETWEEN_ISOBARIC : 
  case GribVertType::BETWEEN_HT_ABOVE_GROUND : 
  case GribVertType::BETWEEN_SIGMA : 
  case GribVertType::BETWEEN_DEPTH : 
  case GribVertType::BETWEEN_PRS_DIFF : 
    _singleLevelValue = false;
    _levelValue = (int) octet10;
    _levelValueBottom = (int) octet11;
    break;

  default:
    cerr << "unknown Level Indicator = " << _vertType << endl;
    _vertType = SURFACE;
    _singleLevelValue = true;
    _levelValue = 0;
    _levelValueBottom = 0;
    break;
  }

}


void GribVertType::print(FILE *stream) const
{
  print(stream, _vertType, _singleLevelValue,
	_levelValue, _levelValue, _levelValueBottom);
}

void GribVertType::print(ostream &stream) const
{
  print(stream, _vertType, _singleLevelValue,
	_levelValue, _levelValue, _levelValueBottom);
}

void GribVertType::print(FILE *stream, const vert_type_t vert_type,
			 const bool is_single_level_value, const int level_val,
			 const int level_val_top, const int level_val_bottom)
{
  fprintf(stream, "vert_type = %d (%s)\n", vert_type,
	  vertType2String(vert_type).c_str());
  if (is_single_level_value)
  {
    fprintf(stream, "level_val = %d\n", level_val);
  }
  else
  {
    fprintf(stream, "level_val_top = %d\n", level_val_top);
    fprintf(stream, "level_val_bottom = %d\n", level_val_bottom);
  }
  
}

void GribVertType::print(ostream &stream, const vert_type_t vert_type,
			 const bool is_single_level_value, const int level_val,
			 const int level_val_top, const int level_val_bottom)
{
  stream << "vert_type = " << vert_type << " ("
	 << vertType2String(vert_type) << ")" << endl;
  
  if (is_single_level_value)
  {
    stream << "level_val = " << level_val << endl;
  }
  else
  {
    stream << "level_val_top = " << level_val_top << endl;
    stream << "level_val_bottom = " << level_val_bottom << endl;
  }
  
}

string GribVertType::vertType2String() const
{
  return vertType2String(_vertType);
}

string GribVertType::vertType2String(vert_type_t vert_type)
{
  switch (vert_type)
  {
  case SURFACE : 
    return "level sfc";
  case CLOUD_BASE :
    return "cld base"; 
  case CLOUD_TOPS : 
    return "cld top"; 
  case ZERO_ISOTHERM : 
    return "OC isotherm"; 
  case MAX_WIND : 
    return "max wind level"; 
  case TROPOPAUSE : 
    return "tropopause"; 
  case ISOBARIC : 
    return "isobaric"; 
  case MEAN_SEA_LEVEL : 
    return "MSL"; 
  case HEIGHT_ABOVE_GROUND : 
    return "height (m) above gound"; 
  case HYBRID : 
    return "hybrid level"; 
  case DEPTH_BELOW_LAND_SURFACE : 
    return "cm below land surface"; 
  case BETWEEN_DEPTH : 
    return "cm between depth"; 
  case BETWEEN_PRS_DIFF : 
    return "mb between pressure diff";
  case POTENTIAL_VORTICITY : 
    return "Potential vorticity surface"; 
  case ENTIRE_ATMOSPHERE :
  case UNKNOWN_10 : 
    return "atmos col"; 
  case FREEZING_LEVEL :
    return "freezing level"; 
  case CONV_CLD_TOP : 
    return "convect-cld top"; 
  case MAX_EQ_THETA_PRESSURE : 
    return "max. eq. theta pressure layer"; 
  case EQUILIBRIUM_LEVEL_HEIGHT : 
    return "equlibrium level height"; 
  default:
    return "unknown vert level type";
  } /* endswitch - vert_type */
  
  return "unknown vert level type";
}

