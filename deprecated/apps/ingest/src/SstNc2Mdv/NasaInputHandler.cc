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
//   $Id: NasaInputHandler.cc,v 1.2 2016/03/07 01:23:05 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * NasaInputHandler: Class for handling the SST input from a NASA file.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include "NasaInputHandler.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

NasaInputHandler::NasaInputHandler(const int num_output_data_pts,
				   const MdvxPjg &output_proj,
				   const bool debug_flag,
				   MsgLog *msg_log) :
  InputHandler(num_output_data_pts, output_proj, debug_flag, msg_log)
{
}

  
/*********************************************************************
 * Destructor
 */

NasaInputHandler::~NasaInputHandler()
{
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

/*********************************************************************
 * _getLatLonData() - Get the lat/lon data from the netCDF file.
 */

void NasaInputHandler::_getLatLonData()
{
  _latData = (fl32 *) _ncLatVar->get_data();
  _lonData = (fl32 *) _ncLonVar->get_data();
}


/*********************************************************************
 * _getTimeFromFilename() - Get the UNIX time for the dataset from the
 *                          filename.
 */

time_t NasaInputHandler::_getTimeFromFilename(const string  &file_path) const
{
  // example path:
  // /d1/sgc/OW/sst_samples/20060312.0511.lev2b_ocean.amsre.a1.sst.ncdf
 
  bool date_time_ok = true;

  // Extract the filename from the path

  string::size_type filename_start;
  string::size_type last_slash = file_path.find_last_of('/');

  if(last_slash != string::npos)
    filename_start = last_slash + 1;
  else
    filename_start = 0;

  string::size_type filename_end = file_path.length() - 1;

  string filename = file_path.substr(filename_start,filename_end);

  // Pull the date and time out of the filename.
  // example filename:
  // 20060312.0511.lev2b_ocean.amsre.a1.sst

  string yyyymmdd_str = filename.substr(0,8);
  string hhii_str     = filename.substr(9,4);

  // Put the time information together into another string and parse it
  // using the library

  string yyyymmddhhiiss_str = yyyymmdd_str + hhii_str + "00";

  DateTime nc_filename_time( yyyymmddhhiiss_str.c_str() );

  time_t return_value = nc_filename_time.utime();

  if (date_time_ok)
    return return_value;
  else
    return -1;
}
