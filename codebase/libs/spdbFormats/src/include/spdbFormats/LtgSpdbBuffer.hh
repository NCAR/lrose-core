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
 *   $Date: 2016/03/03 19:25:27 $
 *   $Id: LtgSpdbBuffer.hh,v 1.4 2016/03/03 19:25:27 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * LtgSpdbBuffer.hh : header file for the LtgSpdbBuffer program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef LtgSpdbBuffer_HH
#define LtgSpdbBuffer_HH

/*
 **************************** includes **********************************
 */

#include <cstdio>

#include <rapformats/ltg.h>
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

class LtgSpdbBuffer
{
 public:

  // Constructor

  LtgSpdbBuffer(bool debug_flag);
  
  // Destructor

  ~LtgSpdbBuffer(void);
  
  // Adds the given strike to the strike buffer.

  void addStrike(LTG_strike_t *strike);
  
  // Writes the current SPDB lightning buffer to the database.

  void writeToDatabase(char *database,
		       int expire_secs,
		       int check_old_data);
  
  // Clear the buffer.

  void clear(void);
  
 private:
  
  // Debug flag

  bool _debugFlag;
  
  // Local buffers

  LTG_strike_t *_strikeBuffer;
  LTG_strike_t *_strikeBufferBE;
  int _strikeBufferUsed;
  int _strikeBufferAlloc;
  
  // Makes sure the strike buffer has enough room to store the
  // indicated number of strikes.

  void _checkBufferAllocation(int num_strikes_needed);
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("LtgSpdbBuffer");
  }
  
};


#endif
