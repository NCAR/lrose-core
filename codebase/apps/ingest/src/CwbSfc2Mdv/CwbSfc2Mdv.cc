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

#include <cstdio>
#include <iostream>
#include <math.h>

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <cstdlib> 

#include <stdlib.h> // For calloc

#include <netcdf.h>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxRemapLut.hh>

#include "CwbSfc2Mdv.hh"
#include "FieldHandler.hh"

using namespace std;


CwbSfc2Mdv::CwbSfc2Mdv( Params *TDRP_params ) :
  _params(TDRP_params)
{

  // Initialize the field handler list

  for (int i = 0; i < _params->input_field_list_n; ++i)
  {
    FieldHandler *field_handler = 
      new FieldHandler(_params->_input_field_list[i],
		       _params->debug >= Params::DEBUG_NORM);
    _fieldHandlerList.push_back(field_handler);
  }
  
}

////////////////////////////////////////////////////
//
// Destructor
//

CwbSfc2Mdv::~CwbSfc2Mdv( )
{
  vector< FieldHandler* >::iterator handler_iter;
  
  for (handler_iter = _fieldHandlerList.begin();
       handler_iter != _fieldHandlerList.end(); ++handler_iter)
    delete *handler_iter;
}


////////////////////////////////////////////////////
//
// Main routine - processes a file.
//

void  CwbSfc2Mdv::CwbSfc2MdvFile( char *filename )
{
  if (_params->debug)
    cerr << endl << "*** Processing file : " << filename << endl;

  // Uncompress the input file, if necessary

  if (ta_file_uncompress(filename) < 0)
  {
    cerr << "Error uncompressing input file: " << filename << endl;
    return;
  }
  
  // Create the output file

  DsMdvx mdvx;

  // Open the netCDF file

  int status, ncID;
  status = nc_open(filename, NC_NOWRITE, &ncID);
  if (status != NC_NOERR)
  {
    cerr << "Failed to open " << filename << endl;
    return;
  }

  // Process each of the fields in the input file

  vector< FieldHandler* >::iterator handler_iter;
  bool first_field = true;
  
  for (handler_iter = _fieldHandlerList.begin();
       handler_iter != _fieldHandlerList.end(); ++handler_iter)
  {
    // Get a pointer to the field handler for easier use

    FieldHandler *field_handler = *handler_iter;

    MdvxField *field;
    field = field_handler->extractField(ncID);
    MdvxRemapLut lut;
    field->autoRemap2Latlon(lut);
    
    // If we couldn't read the field properly for some reason, skip it

    if (field == 0)
      continue;
  
    PMU_force_register("converting data");

    Mdvx::encoding_type_t encoding = (Mdvx::encoding_type_t)_params->encoding_type;
    Mdvx::compression_type_t compression = (Mdvx::compression_type_t)_params->compression_type;
    
    if( encoding != Mdvx::ENCODING_FLOAT32)
    {
      if ( field->convertRounded(encoding, compression) )
      {
	cerr << "convertRounded failed for field "
	     << field_handler->getFieldName() << endl;
	cerr << field->getErrStr() << endl;
	continue;
      }
    }
    else
    {
      if (field->convertType(encoding, compression) ) 
      {
	cerr << "convertType failed for field " 
	     << field_handler->getFieldName() << endl;
	cerr << field->getErrStr() << endl;
      }
    }

    mdvx.addField(field);

    if (first_field)
    {
      _updateMasterHeader(mdvx, *field, filename);
      first_field = false;
    }
    
  }
  
  // Close the netcdf file.

  nc_close(ncID);     

  // Make sure we were able to successfully process a field

  if (first_field)
  {
    cerr << "Couldn't process any fields successfully -- skipping file" << endl;
    return;
  }
  
  if (_params->debug)
    cerr << endl << "Writing file to url: " << _params->outUrl << endl;

  if (_params->writeLdataInfo)
    mdvx.setWriteLdataInfo();
  else
    mdvx.clearWriteLdataInfo();

  if (mdvx.writeToDir(_params->outUrl))
  {
    cerr << "ERROR - Output::write" << endl;
    cerr << "  Cannot write output." << endl;
    cerr << mdvx.getErrStr() << endl;
    return;
  }
   

  return;


}


/**********************************************************************
 * _updateMasterHeader() - Update the master header for the output file
 *                         based on the information in this field.
 */

void CwbSfc2Mdv::_updateMasterHeader(DsMdvx &mdv_file,
					 const MdvxField &field,
					 const string &filename)
{
  Mdvx::master_header_t master_hdr = mdv_file.getMasterHeader();
  
  master_hdr.time_gen = time(NULL);
  
  master_hdr.time_begin = field.getFieldHeader().forecast_time;
  master_hdr.time_end = master_hdr.time_begin;
  master_hdr.time_centroid = master_hdr.time_begin;
  master_hdr.time_expire = master_hdr.time_begin + _params->Expiry;

  if (field.getFieldHeader().nz > 1)
    master_hdr.data_dimension = 3;
  else
    master_hdr.data_dimension = 2;
  
  master_hdr.data_collection_type = Mdvx::DATA_MEASURED;
  master_hdr.grid_orientation = Mdvx::ORIENT_NS_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;

  master_hdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  master_hdr.vlevel_included = 1;

  sprintf(master_hdr.data_set_info,"%s","NSSL radar mosaic");
  sprintf(master_hdr.data_set_name,"%s","NSSL radar mosaic");
  sprintf(master_hdr.data_set_source,"%s", filename.c_str());

  mdv_file.setMasterHeader(master_hdr); 
}
