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
 *   $Date: 2016/03/06 23:53:41 $
 *   $Id: Mdv2Kavm.hh,v 1.3 2016/03/06 23:53:41 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Mdv2Kavm.hh : header file for the Mdv2Kavm program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2001
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Mdv2Kavm_HH
#define Mdv2Kavm_HH

/*
 **************************** includes **********************************
 */

#include <sys/time.h>

#include <Mdv/DsMdvxInput.hh>

#include "Args.hh"
#include "Params.hh"
using namespace std;


/*
 ******************************* defines ********************************
 */


/*
 ******************************* structures *****************************
 */

/*
 ************************* global variables *****************************
 */

/*
 ***************************** function prototypes **********************
 */

/*
 ************************* class definitions ****************************
 */

class Mdv2Kavm
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  // Flag indicating whether the program status is currently okay.

  bool okay;


  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Destructor
   */

  ~Mdv2Kavm(void);
  
  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static Mdv2Kavm *Inst(int argc, char **argv);
  static Mdv2Kavm *Inst();
  

  /////////////////////
  // Running methods //
  /////////////////////

  /*********************************************************************
   * run() - run the program.
   */

  void run();
  

 private:

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static Mdv2Kavm *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  

  // Input file retriever object

  DsMdvxInput _fileRetriever;
  


  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  Mdv2Kavm(int argc, char **argv);
  
  /*********************************************************************
   * _className() - Return the class name for error messages.
   */

  static const char *_className(void)
  {
    return("Mdv2Kavm");
  }
  
};


#endif
