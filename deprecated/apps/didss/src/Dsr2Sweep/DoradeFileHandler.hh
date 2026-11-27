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
/////////////////////////////////////////////////////////////
// DoradeFileHandler - Class for handling the DORADE sweep files.
//
// Nancy Rehak, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2006
//
///////////////////////////////////////////////////////////////

#ifndef DoradeFileHandler_H
#define DoradeFileHandler_H

#include <DoradeFile.h>
#include <DoradeFileName.h>

#include "FileHandler.hh"

using namespace std;


class DoradeFileHandler : public FileHandler
{
  
public:

  /*********************************************************************
   * Constructors
   */
  
  DoradeFileHandler(const Params &params);

  /*********************************************************************
   * Destructor
   */
  
  virtual ~DoradeFileHandler();


  /*********************************************************************
   * generateFileName() - Generate the appropriate name for the sweep file.
   */
  
  virtual string generateFileName(const ForayUtility::RaycTime &start_time,
				  const int scan_type,
				  const double fixed_angle,
				  const int volume_number,
				  const int sweep_number);

protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  DoradeFileName file_namer;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _createFileObj() - Create the appropriate file object.
   */

  virtual RayFile *_createFileObj()
  {
    return new DoradeFile();
  }


};

#endif

