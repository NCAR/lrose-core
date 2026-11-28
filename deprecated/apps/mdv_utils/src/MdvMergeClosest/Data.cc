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
#include <toolsa/copyright.h>

/**
 * @file Data.cc
 */

#include "Data.hh"
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/LogMsg.hh>
#include <toolsa/DateTime.hh>

//----------------------------------------------------------------
Data::Data(const ParmInput &dparm, const Parms &parm) :
  _dataParm(dparm), _parm(parm), _projectionIsSet(false), _time(0)
{
}

//----------------------------------------------------------------
Data::~Data()
{
}

//----------------------------------------------------------------
void Data::print(void) const
{
  LOGF(LogMsg::DEBUG, " Url:%s  time:%s", _dataParm._url.c_str(),
       DateTime::strn(_time).c_str());
}

//----------------------------------------------------------------
bool Data::read(DsMdvx &in)
{
  in.setDebug(_parm._debugVerbose);
  in.clearRead();
  in.setReadTime(Mdvx::READ_FIRST_BEFORE, _dataParm._url, 0, _time);

  for (size_t ii = 0; ii < _parm._field.size(); ii++)
  {
    in.addReadField(_parm._field[ii]);
    LOGF(LogMsg::DEBUG_VERBOSE, "Reading field name: %s",
	 _parm._field[ii].c_str());
  }
  in.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  in.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  // perform the read
  if (in.readVolume())
  {
    LOGF(LogMsg::ERROR, "Cannot read data for url: %s", _dataParm._url.c_str());
    return false;
  }

  Mdvx::master_header_t mhdr = in.getMasterHeader();
  if (mhdr.data_dimension == 3 && mhdr.max_nz != 1)
  {
    LOG(LogMsg::ERROR, "Not a 2 dimensional array");
    return false;
  }

  bool status = true;
  for (size_t i=0; i<_parm._field.size(); ++i)
  {
    if (!_setField(in, mhdr, _parm._field[i]))
    {
      status = false;
    }
  }
  if (status == false)
  {
    return false;
  }
  return true;
}

//----------------------------------------------------------------
bool Data::_setField(DsMdvx &d, const Mdvx::master_header_t &mhdr,
		     const std::string &name)
{
  MdvxField *f = d.getFieldByName(name);
  if (f == NULL)
  {
    LOGF(LogMsg::ERROR, "reading field %s", name.c_str());
    return false;
  }

  Mdvx::field_header_t hdr = f->getFieldHeader();
  MdvxProj proj(mhdr, hdr);
  if (!_projectionIsSet)
  {
    _projectionIsSet = true;
    _proj = proj;
  }
  else
  {
    if (_proj != proj)
    {
      LOG(LogMsg::ERROR, "projection has changed, not allowed");
      return false;
    }
  }
  string units = hdr.units;
  fl32 *data = (fl32 *)f->getVol();
  Grid2d gr(name, hdr.nx, hdr.ny, hdr.missing_data_value);
  for (int j=0; j<hdr.nx*hdr.ny*hdr.nz ; ++j)
  {
    if (data[j] != hdr.bad_data_value && data[j] != hdr.missing_data_value)
    {
      gr.setValue(j, static_cast<double>(data[j]));
    }
  }
  _grid[name] = gr;
  return true;
}

//----------------------------------------------------------------
void Data::setAllMissing(void)
{
  map<string,Grid2d>::iterator i;
  for (i=_grid.begin(); i!=_grid.end(); ++i)
  {
    i->second.setAllMissing();
  }
}

//----------------------------------------------------------------
void Data::setValues(const Data &d, int x, int y)
{
  map<string,Grid2d>::iterator i;
  for (i=_grid.begin(); i!=_grid.end(); ++i)
  {
    string name = i->first;
    i->second.setValue(x, y, d._grid.at(name).getValue(x, y));
  }
}
