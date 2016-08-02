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
 * @file OutputData.cc
 */

#include "OutputData.hh"
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/LogMsg.hh>
#include <toolsa/DateTime.hh>

//----------------------------------------------------------------
OutputData::OutputData(const Parms &parm) : Data(ParmInput(), parm)
{
}

//----------------------------------------------------------------
OutputData::~OutputData()
{
}

//----------------------------------------------------------------
void OutputData::init(const Data &data)
{
  // make a copy of the base class into local space
  // note that it sets the URL to the input URL
  Data::operator=(data);

  // tell the data to provide a DsMdvx object by rereading the data
  // from the URL that is in the input
  if (!Data::read(_mdv))
  {
    LOGF(LogMsg::ERROR, "Initialization failed, could not read from %s",
	 data.getUrl().c_str());
    return;
  }

  // set all data to missing
  Data::setAllMissing();

  // change the URL locally back to what it should be
  _dataParm._url = _parm._outputUrl;
}

//----------------------------------------------------------------
void OutputData::write(const time_t &t)
{
  // copy all the fields into the MDV data
  for (int i=0; i<_mdv.getNFields(); ++i)
  {
    // make sure it is the one we think it is
    string name = _mdv.getFieldName(i);
    if (name != _parm._field[i])
    {
      LOGF(LogMsg::ERROR, "Mismatch in MDV and local state %s %s",
	   name.c_str(), _parm._field[i].c_str());
      LOG(LogMsg::ERROR, "NO WRITE");
      return;
    }

    MdvxField *f = _mdv.getFieldByNum(i);
    fl32 *data = (fl32 *)(f->getVol());
    for (int j=0; j<_grid.at(name).getNdata(); ++j)
    {
      data[j] = _grid.at(name).getValue(j);
    }
  }    

  // change time
  Mdvx::master_header_t m = _mdv.getMasterHeader();
  m.time_gen = t;
  m.time_begin = t;
  m.time_end = t;
  m.time_centroid = t;
  m.time_expire = t;
  _mdv.setMasterHeader(m);
  
  // write out
  _mdv.setWriteLdataInfo();
  if (_mdv.writeToDir(_parm._outputUrl))
  {
    LOGF(LogMsg::ERROR, "Writing to %s", _parm._outputUrl.size());
  }
  else
  {
    LOGF(LogMsg::DEBUG, "Wrote to %s, time=%s", _parm._outputUrl.c_str(),
	 DateTime::strn(t).c_str());
  }
}
