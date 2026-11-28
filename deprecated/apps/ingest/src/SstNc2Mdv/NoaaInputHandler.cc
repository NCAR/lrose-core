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
//   $Id: NoaaInputHandler.cc,v 1.2 2016/03/07 01:23:05 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * NoaaInputHandler: Class for handling the NOAA SST input.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include "NoaaInputHandler.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

NoaaInputHandler::NoaaInputHandler(const int num_output_data_pts,
				   const MdvxPjg &output_proj,
				   const bool debug_flag,
				   MsgLog *msg_log) :
  InputHandler(num_output_data_pts, output_proj, debug_flag, msg_log)
{
}

  
/*********************************************************************
 * Destructor
 */

NoaaInputHandler::~NoaaInputHandler()
{
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

/*********************************************************************
 * _getLatLonData() - Get the lat/lon data from the netCDF file.
 */

void NoaaInputHandler::_getLatLonData()
{
  double *inp_lat_1d = NULL;
  double *inp_lon_1d = NULL;
  double inp_lon;
 
  int num_data_pts = _numInputRows * _numInputCols;
  
  _latData = new float [ num_data_pts ];
  _lonData = new float [ num_data_pts ];

  inp_lat_1d = (double *) _ncLatVar->get_data();
  inp_lon_1d = (double *) _ncLonVar->get_data();

  _latData1D = new float [ _numInputRows ];
  _lonData1D = new float [ _numInputCols ];

  for (int i_row = 0; i_row < _numInputRows; ++i_row)
  {
    for (int j_col = 0; j_col < _numInputCols; ++j_col)
    {
      int indx_2d = i_row * _numInputCols + j_col;

      inp_lon = inp_lon_1d[ j_col ];
      if (inp_lon > 180.0)
	inp_lon = inp_lon - 360.0;

      _latData1D[ i_row ] = (float) inp_lat_1d[ i_row ];
      _lonData1D[ j_col ] = (float) inp_lon;

      _latData[ indx_2d ] = (float) inp_lat_1d[ i_row ];
      _lonData[ indx_2d ] = (float) inp_lon;
    } /* endfor - j_col */
  } /* endfor - i_row */

}


/*********************************************************************
 * _getTimeFromFilename() - Get the UNIX time for the dataset from the
 *                          filename.
 */

time_t NoaaInputHandler::_getTimeFromFilename(const string  &file_path) const
{
  // example path:
  // /d1/sgc/OW/sst_samples/NOAA/2006-11-24_00:00:00_2006-11-25_23:59:59-SST50-anal_temp--80.0_85.0_0.0_360.0-.5.NETCDF
 
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

  // example filename:
  // 0                  20   25 28 31 34 37
  // |                   |    |  |  |  |  |
  // 2006-11-24_00:00:00_2006-11-25_23:59:59-SST50-anal_temp--80.0_85.0_0.0_360.0-.5.NETCDF
  
  string yyyy_str = filename.substr(20,4);
  string mm_str   = filename.substr(25,2);
  string dd_str   = filename.substr(28,2);
  string hh_str   = filename.substr(31,2);
  string ii_str   = filename.substr(34,2);
  string ss_str   = filename.substr(37,2);

  // Put the time information together into another string and parse it
  // using the library

  string yyyymmddhhiiss_str =
    yyyy_str + mm_str + dd_str + hh_str + ii_str + ss_str;

  DateTime nc_filename_time(yyyymmddhhiiss_str.c_str());
 
  time_t return_value = nc_filename_time.utime();

  if (date_time_ok)
    return return_value;
  else
    return FAILURE;

}
