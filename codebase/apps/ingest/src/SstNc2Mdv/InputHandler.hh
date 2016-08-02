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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/07 01:23:05 $
 *   $Id: InputHandler.hh,v 1.2 2016/03/07 01:23:05 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * InputHandler: Base class for classes that handle the SST input.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2007
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef InputHandler_H
#define InputHandler_H

#include <string>
#include <vector>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/MsgLog.hh>

#include "InputVar.hh"
#include "NetcdfDataset.hh"
#include "NetcdfUtils.hh"

using namespace std;


class InputHandler
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  InputHandler(const int num_output_data_pts,
	       const MdvxPjg &output_proj,
	       const bool debug_flag,
	       MsgLog *msg_log);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~InputHandler();


  ///////////////////
  // Input Methods //
  ///////////////////

  /*********************************************************************
   * readData() - Read the indicated variables from the given netCDF file.
   *
   * Returns the data time of the read data on success, -1 on failure.
   */

  time_t readData(const string &input_path,
		  const string &lat_field_name,
		  const string &lat_missing_att_name,
		  const string &lon_field_name,
		  const string &lon_missing_att_name,
		  const string &rows_dim_name,
		  const string &cols_dim_name);
  

  /*********************************************************************
   * createMdvFields() - Create the requested MDV fields from the netCDF
   *                     file information.
   *
   * Returns true on success, false on failure.
   */

  bool createMdvFields(DsMdvx &mdv_file);
  

  ////////////////////
  // Access Methods //
  ////////////////////

  /*********************************************************************
   * addInputVar() - Add an input variable to the list to be processed.
   */

  void addInputVar(const InputVar &input_var)
  {
    _inputVarList.push_back(input_var);
  }


  /*********************************************************************
   * GetMsgLog() - Get the message log.  This method is required for use
   *               of the POSTMSG macro.
   */

  MsgLog &GetMsgLog()
  {
    return *_msgLog;
  }


protected:

  /////////////////////////
  // Protected Constants //
  /////////////////////////

  static const double LAT_MISSING_VALUE;
  static const double LON_MISSING_VALUE;
  

  ///////////////////////
  // Protected Members //
  ///////////////////////

  bool _debug;
  MsgLog *_msgLog;

  vector< InputVar > _inputVarList;
  
  int _numOutputDataPts;
  MdvxPjg _projection;
  
  time_t _latestDataTime;
  
  fl32 _latMissingValue;
  fl32 _lonMissingValue;
  
  int _numInputRows;
  int _numInputCols;

  NcdVar *_ncLatVar;
  NcdVar *_ncLonVar;
  
  fl32 *_latData;
  fl32 *_lonData;
  fl32 *_sstData;
  

  ///////////////////////
  // Protected Methods //
  ///////////////////////

  /*********************************************************************
   * _getLatLonData() - Get the lat/lon data from the netCDF file.
   */

  virtual void _getLatLonData() = 0;
  

  /*********************************************************************
   * _getTimeFromFilename() - Get the UNIX time for the dataset from the
   *                          filename.
   */

  virtual time_t _getTimeFromFilename(const string  &file_path) const = 0;
  
};

#endif
