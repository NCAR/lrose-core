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
 *   $Date: 2016/03/04 02:22:09 $
 *   $Id: Binary2Mdv.hh,v 1.2 2016/03/04 02:22:09 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Binary2Mdv.hh : header file for the Binary2Mdv program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2004
 *
 * Dan Megenhardt
 *
 ************************************************************************/

#ifndef Binary2Mdv_HH
#define Binary2Mdv_HH

#include <dataport/port_types.h>
#include <toolsa/InputDir.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


class Binary2Mdv
{
 public:

  // Destructor

  ~Binary2Mdv(void);
  
  // Get Binary2Mdv singleton instance

  static Binary2Mdv *Inst(int argc, char **argv);
  static Binary2Mdv *Inst();
  
  // Run the program.

  void run();
  
  // Flag indicating whether the program status is currently okay.

  bool okay;
  
 private:

  InputDir *_inputDir;

  // Constructor -- private because this is a singleton object

  Binary2Mdv(int argc, char **argv);
  
  // Singleton instance pointer

  static Binary2Mdv *_instance;
  
  // Define private null copy constructor and assignment operator so the
  // default ones are not accidentally used.

  Binary2Mdv(const Binary2Mdv & other);
  Binary2Mdv & operator= (const Binary2Mdv & other);
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  fl32 *_data;
    
  int writeMDV(const string &input_file_name);
  char *_getNextFilename(void);
  bool processFile(const string &filename);
  bool _readFile(const string &filename);
  DateTime getDataTime(const string &input_file_name);
   
  
};


#endif
