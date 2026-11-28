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
 *   $Date: 2016/03/04 02:22:16 $
 *   $Id: Unisysm2Mdv.hh,v 1.3 2016/03/04 02:22:16 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Unisysm2Mdv.hh : header file for the Unisysm2Mdv program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2003
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Unisysm2Mdv_HH
#define Unisysm2Mdv_HH

#include <dataport/port_types.h>
#include <toolsa/InputDir.hh>

#include "Args.hh"
#include "Params.hh"
#include "UnisysFile.hh"

using namespace std;


class Unisysm2Mdv
{
 public:

  // Destructor

  ~Unisysm2Mdv(void);
  
  // Get Unisysm2Mdv singleton instance

  static Unisysm2Mdv *Inst(int argc, char **argv);
  static Unisysm2Mdv *Inst();
  
  // Run the program.

  void run();
  
  // Flag indicating whether the program status is currently okay.

  bool okay;
  
 private:

  InputDir *_inputDir;

  // Constructor -- private because this is a singleton object

  Unisysm2Mdv(int argc, char **argv);
  
  // Singleton instance pointer

  static Unisysm2Mdv *_instance;
  
  // Define private null copy constructor and assignment operator so the
  // default ones are not accidentally used.

  Unisysm2Mdv(const Unisysm2Mdv & other);
  Unisysm2Mdv & operator= (const Unisysm2Mdv & other);
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  UnisysFile _unisysFile;
  
  MdvxRemapLut _remapLut;
  
  int writeMDV(const string &input_file_name);
  char *_getNextFilename(void);
  int processFile(const string &filename);
  
};


#endif
