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
// TstormSpdb2Ascii.h
//
// TstormSpdb2Ascii object
//
// Dan Megenhardt, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2005
//
///////////////////////////////////////////////////////////////

#ifndef TstormSpdb2Ascii_H
#define TstormSpdb2Ascii_H

#include <stdio.h>

#include <toolsa/os_config.h>
#include <symprod/spdb.h>
#include <tdrp/tdrp.h>
#include <rapformats/tstorm_spdb.h>
#include <toolsa/ldata_info.h>
#include <toolsa/pmu.h>

#include "Args.hh"
#include "Params.hh"


class TstormSpdb2Ascii
{
  
public:

  // destructor
  
  ~TstormSpdb2Ascii();

  // Get TstormSpdb2Ascii singleton instance

  static TstormSpdb2Ascii *Inst(int argc, char **argv);
  static TstormSpdb2Ascii *Inst();
  
  // run 

  void run();

  // data members

  bool okay;

protected:
  
private:

  // constructor -- private because this is a singleton object

  TstormSpdb2Ascii(int argc, char **argv);

  // Singleton instance pointer

  static TstormSpdb2Ascii *_instance;
  
  char *_programName;
  Args *_args;
  Params *_params;
  
  time_t _archiveStartTime;
  time_t _archiveEndTime;
  
  LDATA_handle_t _ldataHandle;
  
  ui32                  _stormDataNchunks;
  spdb_chunk_ref_t     *_stormDataChunkHdrs;
  char                 *_stormData;
  
  int _getStormData(void);
  
  time_t _retrieveArchiveStormData(time_t start_time,
				   time_t end_time);

  int _retrieveStormData(time_t data_time);
  
  void _convertToAscii(void);
  
  void _printHeader(FILE *stream);
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("TstormSpdb2Ascii");
  }
  
};

#endif
