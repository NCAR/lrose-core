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

/************************************************************************
 * UpdateMdvCollectionType: UpdateMdvCollectionType program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2007
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef UpdateMdvCollectionType_HH
#define UpdateMdvCollectionType_HH

#include <sys/time.h>

#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvx.hh>

#include "Args.hh"
#include "Params.hh"
using namespace std;

class UpdateMdvCollectionType
{
 public:

  // Destructor

  ~UpdateMdvCollectionType(void);
  
  // Get UpdateMdvCollectionType singleton instance

  static UpdateMdvCollectionType *Inst(int argc, char **argv);
  static UpdateMdvCollectionType *Inst();
  
  // Run the program.

  int run();
  
  // Flag indicating whether the program status is currently okay.

  bool okay;
  
  // Retrieves the program parameters

  Params *getParams(void)
  {
    return(_params);
  }
  
 private:

  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;

  DsTrigger *_trigger;
  
  Mdvx::data_collection_type_t _dataCollectionType;
  
  // Constructor -- private because this is a singleton object

  UpdateMdvCollectionType(int argc, char **argv);
  
  // Singleton instance pointer

  static UpdateMdvCollectionType *_instance;

  // process file

  int _processFile(const string &file_path);
  int _processFile(const DateTime &data_time);

  int _readAndProcessFile(DsMdvx &mdvx);
  
};


#endif
