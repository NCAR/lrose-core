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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 01:23:05 $
//   $Id: InputHandler.cc,v 1.4 2016/03/07 01:23:05 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * InputHandler: Base class for classes that handle the SST input.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include "InputHandler.hh"

using namespace std;

const double InputHandler::LAT_MISSING_VALUE = -999.0;
const double InputHandler::LON_MISSING_VALUE = -999.0;


/*********************************************************************
 * Constructors
 */

InputHandler::InputHandler(const int num_output_data_pts,
			   const MdvxPjg &output_proj,
			   const bool debug_flag,
			   MsgLog *msg_log) :
  _debug(debug_flag),
  _msgLog(msg_log),
  _numOutputDataPts(num_output_data_pts),
  _projection(output_proj)
{
}

  
/*********************************************************************
 * Destructor
 */

InputHandler::~InputHandler()
{
}


/*********************************************************************
 * createMdvFields() - Create the requested MDV fields from the netCDF
 *                     file information.
 *
 * Returns true on success, false on failure.
 */

bool InputHandler::createMdvFields(DsMdvx &mdv_file)
{
  vector< InputVar >::const_iterator input_var;
  
  for (input_var = _inputVarList.begin(); input_var != _inputVarList.end();
       ++input_var)
  {
    POSTMSG( INFO, "Processing input variable....");
 
    MdvxField *field = input_var->createMdvxField(_projection,
						  _latestDataTime,
						  _latData, _lonData);
  
    if (field == 0)
      return false;
  
    mdv_file.addField(field);
  } /* endfor - input_var */
  
  return true;
}


/*********************************************************************
 * readData() - Read the indicated variables from the given netCDF file.
 *
 * Returns the data time of the read data on success, -1 on failure.
 */

time_t InputHandler::readData(const string &input_path,
			      const string &lat_field_name,
			      const string &lat_missing_att_name,
			      const string &lon_field_name,
			      const string &lon_missing_att_name,
			      const string &rows_dim_name,
			      const string &cols_dim_name)
{
  const string METHOD_NAME = "InputHandler::ReadInpSSTData()";
  
  // Initialize the mNcuPrtArgs struct
  // This is used as an argument to
  //    NetcdfUtils::ReadNcVar
  //    NetcdfUtils::LoadNcVar
  //    (etc)
  // to give access to the diagnostic output file

  NcuPrtArgs_t nc_print_args;
 
  nc_print_args.callerID = METHOD_NAME;
  nc_print_args.log      = _msgLog;
  nc_print_args.diag     = 0;
  nc_print_args.diag_lvl = 0;
  
  // Create a new NetcdfDataset object to hold the input SST data

  NetcdfDataset *nc_dataset = new NetcdfDataset( nc_print_args );

  // parse the time from the Netcdf filename; convert to Unix time

  time_t data_time = _getTimeFromFilename(input_path);

  if (data_time == FAILURE)
  {
    POSTMSG(ERROR, "Unable to parse date/time from '%s'",
	    input_path.c_str() );
    return -1;
  }

  // Read the input SST Netcdf file

  vector< string > input_var_list;
  
  vector< InputVar >::const_iterator input_var_iter;
  for (input_var_iter = _inputVarList.begin();
       input_var_iter != _inputVarList.end(); ++input_var_iter)
    input_var_list.push_back(input_var_iter->getNcVarName());
  input_var_list.push_back(lat_field_name);
  input_var_list.push_back(lon_field_name);
  
  if (nc_dataset->LoadNetcdfFile(input_path, &input_var_list) != SUCCESS)
  {
    POSTMSG(ERROR, "Error reading Netcdf input file '%s'",
	    input_path.c_str() );
    return -1;
  }

  // Get variables and missing values from input SST object
  //
  // NOTE: NOAA SST files do not have a missing value attribute for their
  // latitude and longitude variables

  _ncLatVar = nc_dataset->get_var(lat_field_name);
  _ncLonVar = nc_dataset->get_var(lon_field_name);

  if (lat_missing_att_name.size() > 0)
  {
    _latMissingValue =
      *((float *) _ncLatVar->get_att(lat_missing_att_name)->values());
  }
  else
  {
    _latMissingValue = LAT_MISSING_VALUE;
  }

  if (lon_missing_att_name.size() > 0)
  {
    _lonMissingValue =
      *((float *) _ncLonVar->get_att(lon_missing_att_name)->values());
  }
  else
  {
    _lonMissingValue = LON_MISSING_VALUE;
  }

  // Get dimensions from input SST object

  NcdDim *input_rows_dim = nc_dataset->get_dim(rows_dim_name);
  NcdDim *input_columns_dim = nc_dataset->get_dim(cols_dim_name);

  _numInputRows    = input_rows_dim->size();
  _numInputCols    = input_columns_dim->size();

  // Get lat and lon data from input SST object
  //
  // If this is a NOAA (global) file, we need to create 2D arrays
  // called "mInpDataLat" and "mInpDataLon" from the 1D arrays found
  // in the Netcdf file. Otherwise, use the 2D lat and lon fields
  // found in the NASA files.

  _getLatLonData();
 
  // Get the field data

  vector< InputVar >::iterator input_var;
  
  for (input_var = _inputVarList.begin();
       input_var != _inputVarList.end(); ++input_var)
  {
    if (!input_var->readData(nc_dataset, _numInputRows, _numInputCols))
    {
      POSTMSG(ERROR, "Error reading data for variable");
      return -1;
    }
  }
  
  return data_time;
}

/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
