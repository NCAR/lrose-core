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
/*********************************************************************
 * WrfVIL2Mdv: WrfVIL2Mdv program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * Feb 2008
 *
 * Dan Megenhardt
 *
 *********************************************************************/

#include <cstdio>
#include <iostream>
#include <math.h>

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <cstdlib> 

#include <stdlib.h>

#include <netcdf.h>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

#include "WrfVIL2Mdv.hh"

#include "WrfVILFieldHandler.hh"

using namespace std;


WrfVIL2Mdv::WrfVIL2Mdv( Params *TDRP_params ) :
  _params(TDRP_params)
{
  // Make sure the parameters are internally consistent

  if (_params->remapToLatlon && _params->remapToFlat)
  {
    cerr << "Only one of remapToLatlon and remapToFlat should ";
    cerr << "be selected in the params file." << endl;
    exit(-1);
  }

  // Initialize the field handler list

  for (int i = 0; i < _params->input_field_list_n; ++i)
  {
    FieldHandler *field_handler;
    
    field_handler =
    new WrfVILFieldHandler(_params->user_defined_bad_missing,
                           _params->bad_missing_val,
			   _params->redefine_bad_missing,
			   _params->new_bad_missing_val,
			   _params->scale_data,
                           _params->multiplicative_factor,
                           _params->_input_field_list[i],
                           _params->debug >= Params::DEBUG_NORM);

    _fieldHandlerList.push_back(field_handler);
  }
  
}

////////////////////////////////////////////////////
//
// Destructor
//

WrfVIL2Mdv::~WrfVIL2Mdv( )
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

void  WrfVIL2Mdv::WrfVIL2MdvFile( char *filename )
{
  if (_params->debug)
    cerr << endl << "*** Processing file : " << filename << endl;

  // Uncompress the input file, if necessary

  if (ta_file_uncompress(filename) < 0)
  {
    cerr << "Error uncompressing input file: " << filename << endl;
    return;
  }
  
  // Open the netCDF file

  int status, ncID;
  status = nc_open(filename, NC_NOWRITE, &ncID);
  if (status != NC_NOERR)
  {
    cerr << "Failed to open " << filename << endl;
    return;
  }

  vector< FieldHandler* >::iterator handler_iter;
  bool first_field;

  vector< time_t > times;
  times = _fieldHandlerList[0]->extractTimes(ncID);
  
// Process each time
  DsMdvx mdvx;

  for(int current_time = 0; current_time < (int)times.size(); current_time++)
  {

    first_field = true;
    mdvx.clearFields();

    for (handler_iter = _fieldHandlerList.begin();
	 handler_iter != _fieldHandlerList.end(); ++handler_iter)
    {

    // Get a pointer to the field handler for easier use

      FieldHandler *field_handler = *handler_iter;
      float *data = field_handler->extractData(ncID);

      MdvxField *field;
      field = field_handler->extractField(ncID,times[current_time], current_time, data);

      if (field == 0)
	continue;
  
      if (_params->remapToLatlon)
      {
	if (field->remap2Latlon(_lut,
				_params->latlonEarthGrid.nx,
				_params->latlonEarthGrid.ny,
				_params->latlonEarthGrid.minx,
				_params->latlonEarthGrid.miny,
				_params->latlonEarthGrid.dx,
				_params->latlonEarthGrid.dy
				)
	    )
	{
	  cerr << "Remap to latlon failed for field"
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

    //      if (_params->debug)
    //	cerr << "converting data to int8\n";
  
    //PMU_force_register("converting data");
    //      if (field->convertRounded(Mdvx::ENCODING_INT8,
    //				Mdvx::COMPRESSION_ZLIB))
    //      {
    //	cerr << "convertRounded failed for field "
    //	     << field_handler->getFieldName() << endl;
    //	continue;
    //      }

      if (_params->debug)
	cerr << endl << "addField" << endl;

      mdvx.addField(field);

      if (first_field)
      {
	_updateMasterHeader(mdvx, *field, filename);
	first_field = false;
      }
      
      delete data;

    }

  // Write the output file.
    if ( _params->writeAsForecast)
      mdvx.setWriteAsForecast();
    
    if (mdvx.writeToDir(_params->outUrl))
    {
      cerr << "ERROR - Output::write" << endl;
      cerr << "  Cannot write output." << endl;
      cerr << mdvx.getErrStr() << endl;
      return;
    }

    if (_params->debug) 
    {
      cerr << endl << "Writing file to url: " << _params->outUrl << endl;
      cerr << "time =  " << times[current_time] << endl;
      cerr << endl;
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

  return;


}


/**********************************************************************
 * _updateMasterHeader() - Update the master header for the output file
 *                         based on the information in this field.
 */

void WrfVIL2Mdv::_updateMasterHeader(DsMdvx &mdv_file,
					 const MdvxField &field,
					 const string &filename)
{
  Mdvx::master_header_t master_hdr = mdv_file.getMasterHeader();
  
  master_hdr.time_gen = field.getFieldHeader().forecast_time - field.getFieldHeader().forecast_delta;
  
  master_hdr.time_begin = field.getFieldHeader().forecast_time;
  master_hdr.time_end = master_hdr.time_begin;
  master_hdr.time_centroid = master_hdr.time_begin;
  master_hdr.time_expire = master_hdr.time_begin + _params->Expiry;

  if (field.getFieldHeader().nz > 1)
    master_hdr.data_dimension = 3;
  else
    master_hdr.data_dimension = 2;
  
  master_hdr.data_collection_type = Mdvx::DATA_FORECAST;
  master_hdr.grid_orientation = Mdvx::ORIENT_NS_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;

  master_hdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  master_hdr.vlevel_included = 1;

  sprintf(master_hdr.data_set_info,"%s","NetCDF Wrf Model Field");
  sprintf(master_hdr.data_set_name,"%s","NetCDF Wrf VIL");
  sprintf(master_hdr.data_set_source,"%s", filename.c_str());

  mdv_file.setMasterHeader(master_hdr); 
}
