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
///////////////////////////////////////////////////
// MdvForHt
//
// Holds data for a single height level
//
// Mike Dixon, EOL, NCAR, Boulder, CO, USA
// April 2015
//
///////////////////////////////////////////////////

#include <math.h>

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/pmu.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

#include "MdvForHt.hh"

/////////////////////////////////////////////
// constructor

MdvForHt::MdvForHt(const Params &params) :
        _params(params)
{

}

/////////////////////////////////////////////
// destructor

MdvForHt::~MdvForHt()
{
  clear();
}

/////////////////////////////////////////////
// clear all data

void MdvForHt::clear() 
{
  _mdvx.clear();
}

/////////////////////////////////////////////
// write the volume

int MdvForHt::writeVol()
{
  
  PMU_auto_register("MdvForHt::writeVol");
  
  if(getNumFields() == 0) {
    cerr << "ERROR: No fields added" << endl << flush;
    return -1;
  }

  // convert field encoding and compression
  
  for (size_t ii = 0; ii < _mdvx.getNFields(); ii++) {
    // _encodeField(_mdvx.getField(ii));
  }

  // always write latest data info
  
  _mdvx.setWriteLdataInfo();

  // Write file

  if (_mdvx.writeToDir(_params.output_url)) {
    cerr << "ERROR: Could not write file to "
         << _params.output_url << endl;
    cerr << _mdvx.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Wrote file: " << _mdvx.getPathInUse() << endl;
  }

  // Clean up

  clear();
  
  return 0;

}

//////////////////////////////////////////////////////////////
// set the master header

void MdvForHt::setMasterHdr(time_t validTime)
{

   Mdvx::master_header_t masterHdr;

   // Clear out master header

   memset(&masterHdr, 0, sizeof(Mdvx::master_header_t));
  
   // Fill the master header

   masterHdr.record_len1     = sizeof(Mdvx::master_header_t);
   masterHdr.struct_id       = Mdvx::MASTER_HEAD_MAGIC_COOKIE;
   masterHdr.revision_number = 1;
   masterHdr.num_data_times  = 1;
   masterHdr.index_number    = 0;
   masterHdr.data_dimension  = 3;
    
   masterHdr.data_collection_type = Mdvx::DATA_FORECAST;
   masterHdr.vlevel_included      = TRUE;
   masterHdr.grid_orientation     = Mdvx::ORIENT_SN_WE;
   masterHdr.data_ordering        = Mdvx::ORDER_XYZ;
   masterHdr.sensor_lon           = 0.0;
   masterHdr.sensor_lat           = 0.0;
   masterHdr.sensor_alt           = 0.0;

   masterHdr.time_gen = validTime;
   masterHdr.time_begin = validTime;
   masterHdr.time_end = validTime;
   masterHdr.time_centroid = validTime;
   masterHdr.time_expire = validTime;

   STRncopy(masterHdr.data_set_info, _params.data_set_info, MDV_INFO_LEN);
   STRncopy(masterHdr.data_set_name, _params.data_set_name, MDV_NAME_LEN);
   STRncopy(masterHdr.data_set_source, _params.data_set_source, MDV_NAME_LEN);

   masterHdr.record_len2    = sizeof(Mdvx::master_header_t);

   _mdvx.setMasterHeader(masterHdr);
   _mdvx.updateMasterHeader();

}

//////////////////////////////////////////////////////////////
// add a field

void MdvForHt::addField(MdvxField *inputField)
{
  _mdvx.addField(inputField);
}

//////////////////////////////////////////////////////////////
// get the height of the layer
// returns -9999 if no layer available

double MdvForHt::getHeightKm() const
{
 
  if (getNumFields() < 1) {
    return -9999.0;
  }
  
  const MdvxField *field = _mdvx.getField(0);
  
  return field->getFieldHeader().grid_minz;

}

//////////////////////////////////////////////////////////////
// get the data time
// returns -9999 if no layer available

time_t MdvForHt::getDataTime() const
{
  
  if (getNumFields() < 1) {
    return -9999;
  }
  
  return _mdvx.getValidTime();
  
}

/////////////////////////////////////////////////////
// convert params encoding type to MDV encoding type

Mdvx::encoding_type_t
  MdvForHt::mdvEncoding(Params::encoding_type_t paramEncoding)

{
  switch(paramEncoding) {
    case Params::ENCODING_INT8:
      return Mdvx::ENCODING_INT8;
    case Params::ENCODING_INT16:
      return Mdvx::ENCODING_INT16;
    case Params::ENCODING_ASIS:
    case Params::ENCODING_FLOAT32:
    default:
      return Mdvx::ENCODING_FLOAT32;
  }
}

