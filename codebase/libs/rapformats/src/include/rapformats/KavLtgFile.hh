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
 *   $Date: 2016/03/03 19:23:53 $
 *   $Id: KavLtgFile.hh,v 1.5 2016/03/03 19:23:53 dixon Exp $
 *   $Revision: 1.5 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * KavLtgFile.hh : header file for the KavLtgFile program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef KavLtgFile_HH
#define KavLtgFile_HH

/*
 **************************** includes **********************************
 */


#include <cstdio>

#include <rapformats/kavltg.h>
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

class KavLtgFile
{
 public:

  // Constructor

  KavLtgFile(bool debug_flag);
  
  // Destructor

  ~KavLtgFile(void);
  
  // Load the given data file into the object.  Wait the given number
  // of seconds before loading the file to make sure the creating
  // process has time to finish writing the file.
  //
  // Returns true if the load was successful, false otherwise.

  bool loadFile(const char *filename,
		int processing_delay = 0);
  
  // Retrieves the next lightning strike from the file.
  //
  // Returns a pointer to the strike data on success, returns NULL on
  // error.

  KAVLTG_strike_t *getNextStrike(void);
  
  // Close the current Kavouras lightning file

  void close(void)
  {
    // do nothing
  }
  
 private:
  
  // Debug flag

  bool _debugFlag;
  
  // Input data

  KAVLTG_strike_t *_strikeBuffer;
  int _strikeBufferUsed;
  int _strikeBufferAlloc;

  int _currentStrike;
  
  // Allocate the internal data buffers based on the number of bytes
  // in the new lightning file.

  void _allocateBuffers(int file_bytes);
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("KavLtgFile");
  }
  
};


#endif
