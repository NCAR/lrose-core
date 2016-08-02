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
//   $Id: NasaRalInputHandler.cc,v 1.2 2016/03/07 01:23:05 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * NasaRalInputHandler: Class for handling the SST input from a NASA file
 *                      that is stored using the RAL directory structure.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include "NasaRalInputHandler.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

NasaRalInputHandler::NasaRalInputHandler(const int num_output_data_pts,
					 const MdvxPjg &output_proj,
					 const bool debug_flag,
					 MsgLog *msg_log) :
  NasaInputHandler(num_output_data_pts, output_proj, debug_flag, msg_log)
{
}

  
/*********************************************************************
 * Destructor
 */

NasaRalInputHandler::~NasaRalInputHandler()
{
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

/*********************************************************************
 * _getTimeFromFilename() - Get the UNIX time for the dataset from the
 *                          filename.
 */

time_t NasaRalInputHandler::_getTimeFromFilename(const string  &file_path) const
{
  // example:
  // ConvNowcast/raw/SST/20060711/154900.nc

  bool date_time_ok = true;

  // Find the location of the last slash in the path since this is where
  // the date ends and the time starts.

  string::size_type hhiiss_start;
  string::size_type yyyymmdd_end;
  string::size_type last_slash = file_path.find_last_of('/');

  if (last_slash != string::npos)
  {
    hhiiss_start = last_slash + 1;
    yyyymmdd_end = last_slash;
  }
  else
  {
    // error
    date_time_ok = false;
  }

  // Now pull out the dated subdirectory

  string::size_type yyyymmdd_start;
  string yyyymmdd_dir_path = file_path.substr(0,yyyymmdd_end);
  last_slash = yyyymmdd_dir_path.find_last_of('/');

  if (last_slash != string::npos)
  {
    yyyymmdd_start = last_slash + 1;
  }
  else
  {
    yyyymmdd_start = 0;
  }

  // Extract the time from the file name

  string hhiiss_str   = file_path.substr(hhiiss_start,6);
  string yyyymmdd_str = yyyymmdd_dir_path.substr(yyyymmdd_start,8);

  // Put the time information together into another string and parse it
  // using the library

  string yyyymmddhhiiss_str = yyyymmdd_str + hhiiss_str;

  DateTime nc_filename_time( yyyymmddhhiiss_str.c_str() );

  time_t return_value = nc_filename_time.utime();

  if(date_time_ok)
    return return_value;
  else
    return FAILURE;
}
