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
////////////////////////////////////////////////////////////////////////////
//  BeamDataFieldParms top-level application class
//
//////////////////////////////////////////////////////////////////////////
#include <rapformats/DsBeamDataFieldParms.hh>
using namespace std;

/*----------------------------------------------------------------*/
DsBeamDataFieldParms::DsBeamDataFieldParms(const float scale,
					   const float bias,
					   const bool is_float,
					   const int missing,
					   const float fmissing)
{
  _scale = scale;
  _bias = bias;
  _is_float = is_float;
  _missing = missing;
  _fmissing = fmissing;
}

/*----------------------------------------------------------------*/
DsBeamDataFieldParms::DsBeamDataFieldParms(const DsBeamDataFieldParms &p)
{
  _scale = p._scale;
  _bias = p._bias;
  _is_float = p._is_float;
  _missing = p._missing;
  _fmissing = p._fmissing;
}

/*----------------------------------------------------------------*/
DsBeamDataFieldParms::~DsBeamDataFieldParms() 
{
}

/*----------------------------------------------------------------*/
void DsBeamDataFieldParms::operator=(const DsBeamDataFieldParms &p)
{
  _scale = p._scale;
  _bias = p._bias;
  _is_float = p._is_float;
  _missing = p._missing;
  _fmissing = p._fmissing;
}

/*----------------------------------------------------------------*/
bool DsBeamDataFieldParms::operator==(const DsBeamDataFieldParms &p) const
{
  return (_scale == p._scale &&
	  _bias == p._bias &&
	  _is_float == p._is_float &&
	  _missing == p._missing &&
	  _fmissing == p._fmissing);
}

/*----------------------------------------------------------------*/
void DsBeamDataFieldParms::print(void) const
{
  printf("Scale:%10.8f  Bias:%10.8f missing:%d  fmissing:%10.8f\n",
	 _scale, _bias, _missing, _fmissing);
}

/*----------------------------------------------------------------*/
float DsBeamDataFieldParms::scaled_value(const float v) const
{
  if (v == _missing)
    return _fmissing;
  else
  {
    if (_is_float)
      return v;
    else
      return v*_scale + _bias;
  }
}

/*----------------------------------------------------------------*/
float DsBeamDataFieldParms::unscaled_value(const float v) const
{
  if (v == _fmissing)
    return (float)_missing;
  else
  {
    if (_is_float)
      return v;
    else
      return (v - _bias)/_scale;
  }
}
