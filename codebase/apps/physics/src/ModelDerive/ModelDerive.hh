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
 *   $Date: 2016/03/06 23:15:37 $
 *   $Revision: 1.2 $
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/************************************************************************
 * ModelDerive: ModelDerive program object. Handles finding input data
 * and passing it on to the file handler.
 *
 * ModelDerive is a program designed to read a gridded data file, 
 * derive user requested variables and output the derived grids with 
 * vertical interpolation if requested.
 *
 * Designed to be easily extendable. Derived variable functions are
 * simple to add as are vertical interpolation functions. Additional
 * input/output file handlers also can be created to handle multiple
 * file formats. 
 *
 * RAP, NCAR, Boulder CO
 * Jason Craig
 * Nov 2007
 *
 ************************************************************************/

#ifndef ModelDerive_HH
#define ModelDerive_HH

#include <sys/time.h>

#include <dsdata/DsTrigger.hh>
#include <dsdata/TriggerInfo.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"
#include "DeriveBase.hh"
#include "ModelDeriveMdv.hh"

using namespace std;

class ModelDerive
{
 public:

  // Flag indicating whether the program status is currently okay.
  bool okay;

  /*********************************************************************
   * Destructor
   */
  ~ModelDerive(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */
  static ModelDerive *Inst(int argc, char **argv);
  static ModelDerive *Inst();
  

  /*********************************************************************
   * init() - Initialize the local data.
   *
   * Returns true if the initialization was successful, false otherwise.
   */
  bool init();
  

  /*********************************************************************
   * run() - run the program.
   */
  int run();
  
 private:

  // Singleton instance pointer

  static ModelDerive *_instance;
  
  // Program parameters.
  Params *_params;
  char *_progName;
  Args *_args;
  
  // Input trigger object
  DsTrigger *_dataTrigger;

  // Input handler object
  ModelDeriveMdv *_fileTypeHandler;

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */
  ModelDerive(int argc, char **argv);
  
};


#endif
