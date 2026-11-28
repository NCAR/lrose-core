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
 *   $Id: NasaInputHandler.hh,v 1.2 2016/03/07 01:23:05 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * NasaInputHandler: Class for handling the SST input from a NASA file.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2007
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef NasaInputHandler_H
#define NasaInputHandler_H

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

#include "InputHandler.hh"

using namespace std;


class NasaInputHandler : public InputHandler
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  NasaInputHandler(const int num_output_data_pts,
		   const MdvxPjg &output_proj,
		   const bool debug_flag,
		   MsgLog *msg_log);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~NasaInputHandler();


protected:

  ///////////////////////
  // Protected Methods //
  ///////////////////////

  /*********************************************************************
   * _getLatLonData() - Get the lat/lon data from the netCDF file.
   */

  virtual void _getLatLonData();
  

  /*********************************************************************
   * _getTimeFromFilename() - Get the UNIX time for the dataset from the
   *                          filename.
   */

  virtual time_t _getTimeFromFilename(const string  &file_path) const;
  
};

#endif
