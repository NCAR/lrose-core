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
// OutputFile - adapted from code by Mike Dixon for
// MM5Ingest
//
// $Id: OutputFile.cc,v 1.2 2016/03/07 01:23:08 dixon Exp $
//
///////////////////////////////////////////////////
#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/pmu.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

#include "OutputFile.hh"
#include "Params.hh"
#include "GfsIngest.hh"
using namespace std;

OutputFile::OutputFile( Params *params )
{
  _paramsPtr    = params;
  _mdvObj       = NULL;
}

OutputFile::~OutputFile()
{
  clear();
}

void
OutputFile::clear() 
{
  delete _mdvObj;
  _mdvObj = 0;
}

int 
OutputFile::writeVol( time_t genTime, time_t leadSecs )
{
  PMU_auto_register("In OutputFile::writeVol");

  if( !_mdvObj ) {
    cerr << "ERROR: No fields added" << endl << flush;
    return( RI_FAILURE );
  }
  

  //
  // Write non forecast style file
  //
  if( _paramsPtr->write_non_forecast ) {
    if( _paramsPtr->non_forecast_timestamp == Params::TIMESTAMP_FCAST_TIME) {
      _setMasterHdr( genTime, leadSecs );
    } 
    else {
      _setMasterHdr( genTime, 0 );
    }
    if( _mdvObj->writeToDir( _paramsPtr->non_forecast_mdv_url ) ) {
      cerr << "ERROR: Could not write file to " << _paramsPtr->non_forecast_mdv_url << endl << flush;
      return( RI_FAILURE );
    }
  }
   
  //
  // Write forecast style file
  //
  if( _paramsPtr->write_forecast ) {
    _setMasterHdr( genTime, leadSecs );
    _mdvObj->setWriteAsForecast();
    if( _mdvObj->writeToDir( _paramsPtr->forecast_mdv_url ) ) {
      cerr << "ERROR: Could not write file to " << _paramsPtr->forecast_mdv_url << endl << flush;
      return( RI_FAILURE );
    }
  }

  //
  // Clean up
  //
  clear();
  
  return( RI_SUCCESS );
}

void 
OutputFile::_setMasterHdr( time_t genTime, time_t leadSecs )
{

   Mdvx::master_header_t masterHdr;

   // 
   // Clear out master header
   //
   memset( (void *) &masterHdr, (int) 0, sizeof(Mdvx::master_header_t) );
  
   //
   // Fill the master header
   //
   masterHdr.record_len1     = sizeof( Mdvx::master_header_t );
   masterHdr.struct_id       = Mdvx::MASTER_HEAD_MAGIC_COOKIE_64;
   masterHdr.revision_number = 1;
   masterHdr.num_data_times  = 1;
   masterHdr.index_number    = 0;
   masterHdr.data_dimension  = 3;
  
   masterHdr.native_vlevel_type = _verticalType;
   masterHdr.vlevel_type        = _verticalType;
  
   masterHdr.data_collection_type = Mdvx::DATA_FORECAST;
   masterHdr.vlevel_included      = TRUE;
   masterHdr.grid_orientation     = Mdvx::ORIENT_SN_WE;
   masterHdr.data_ordering        = Mdvx::ORDER_XYZ;
   masterHdr.sensor_lon           = 0.0;
   masterHdr.sensor_lat           = 0.0;
   masterHdr.sensor_alt           = 0.0;

   masterHdr.time_gen      = genTime;
   masterHdr.time_begin    = genTime + leadSecs;
   masterHdr.time_end      = genTime + leadSecs;
   masterHdr.time_centroid = genTime + leadSecs;
   masterHdr.time_expire   = masterHdr.time_centroid
      + (masterHdr.time_end - masterHdr.time_begin);

   STRncopy(masterHdr.data_set_info, _paramsPtr->data_set_info, MDV_INFO_LEN);
   STRncopy(masterHdr.data_set_name, _paramsPtr->data_set_name, MDV_NAME_LEN);
   STRncopy(masterHdr.data_set_source, _paramsPtr->data_set_source, MDV_NAME_LEN);

   masterHdr.record_len2    = sizeof( Mdvx::master_header_t );

   _mdvObj->setMasterHeader( masterHdr );
   _mdvObj->updateMasterHeader();
}

void
OutputFile::addField( MdvxField *inputField )
{
   //
   // Create Mdvx object if necessary
   //
   if( !_mdvObj ) {
      _mdvObj = new DsMdvx();
   }

   //
   // Convert to byte data
   //
   //inputField->convertRounded( Mdvx::ENCODING_INT8, Mdvx::COMPRESSION_RLE );
   // inputField->convertDynamic( Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_RLE );
   inputField->convertType( Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_RLE );

   //
   // Add the field to the Mdvx object
   //
   _mdvObj->addField( inputField );


}

