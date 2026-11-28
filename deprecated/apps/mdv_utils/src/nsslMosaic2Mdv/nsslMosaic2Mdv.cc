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

#include "nsslMosaic2Mdv.hh"

#include "CiwsFieldHandler.hh"
#include "ConusFieldHandler.hh"

using namespace std;


nsslMosaic2Mdv::nsslMosaic2Mdv( Params *TDRP_params ) :
  _params(TDRP_params)
{
  // Make sure the parameters are internally consistent

  if (_params->remapToLambert && _params->remapToFlat)
  {
    cerr << "Only one of remapToLambert and remapToFlat should ";
    cerr << "be selected in the params file." << endl;
    exit(-1);
  }

  // Initialize the field handler list

  for (int i = 0; i < _params->input_field_list_n; ++i)
  {
    FieldHandler *field_handler;
    
    switch (_params->input_file_type)
    {
    case Params::CIWS_TYPE_INPUT_FILE :
      field_handler =
	new CiwsFieldHandler("rr", _params->debug >= Params::DEBUG_NORM);
      break;
      
    case Params::CONUS_TYPE_INPUT_FILE :
      field_handler =
	new ConusFieldHandler(_params->_input_field_list[i],
			      _params->debug >= Params::DEBUG_NORM,
			      _params->format == Params::OLD_FORMAT);
      break;
    }
    
    _fieldHandlerList.push_back(field_handler);
  }
  
}

////////////////////////////////////////////////////
//
// Destructor
//

nsslMosaic2Mdv::~nsslMosaic2Mdv( )
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

void  nsslMosaic2Mdv::nsslMosaic2MdvFile( char *filename )
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
    if(_params->format == Params::OLD_FORMAT)
	field = field_handler->extractField(ncID);
    else	
	field = field_handler->extractFieldWDSS2(ncID);

    // If we couldn't read the field properly for some reason, skip it

    if (field == 0)
      continue;
  
    if (_params->remapToLambert)
    {
      if (field->remap2Lc2(_lut,
			   _params->lambertEarthGrid.nx,
			   _params->lambertEarthGrid.ny,
			   _params->lambertEarthGrid.minx,
			   _params->lambertEarthGrid.miny,
			   _params->lambertEarthGrid.dx,
			   _params->lambertEarthGrid.dy,
			   _params->lambertEarthGrid.origin_lat,
			   _params->lambertEarthGrid.origin_lon,
			   _params->lambertEarthGrid.lat1,
			   _params->lambertEarthGrid.lat2))
      {
	cerr << "Remap to lambert failed for field"
	     << field_handler->getFieldName() << endl;
	continue;
      }
    }
    else if (_params->remapToFlat)
    {
      if (field->remap2Flat(_lut,
			    _params->flatEarthGrid.nx,
			    _params->flatEarthGrid.ny,
			    _params->flatEarthGrid.minx,
			    _params->flatEarthGrid.miny,
			    _params->flatEarthGrid.dx,
			    _params->flatEarthGrid.dy,
			    _params->flatEarthGrid.origin_lat,
			    _params->flatEarthGrid.origin_lon,
			    _params->flatEarthGrid.rotation))
      {
	cerr << "Remap to flat earth failed for field "
	     << field_handler->getFieldName() << endl;
	continue;
      }
    }

    if (_params->debug)
	cerr << "converting data to int8\n";
  
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

    if (_params->debug)
      cerr << endl << "addField" << endl;

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

void nsslMosaic2Mdv::_updateMasterHeader(DsMdvx &mdv_file,
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
