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
////////////////////////////////////////////////////////////////////////
// NcFileHandler - Class for handling the netCDF sweep files.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2006
//
///////////////////////////////////////////////////////////////////////

#include "NcFileHandler.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

NcFileHandler::NcFileHandler(const Params &params) :
        FileHandler(params)
{
  _dataType = "nc";
}


/*********************************************************************
 * Destructor
 */

NcFileHandler::~NcFileHandler()
{
  // Do nothing
}


/*********************************************************************
 * generateFileName() - Generate the appropriate name for the sweep file.
 */

string NcFileHandler::generateFileName
  (const ForayUtility::RaycTime &start_time,
   const int scan_type,
   const double fixed_angle,
   const int volume_number,
   const int sweep_number)

{
  return file_namer.generate_sweep_name(start_time,
                                        _params.platform_name,
                                        _getForayScanMode(scan_type),
                                        fixed_angle,
                                        volume_number,
                                        sweep_number);
}

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
