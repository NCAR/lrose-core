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

  LtgSpdbBuffer(bool debug_flag = false,
		bool put_mode_unique = false);
  
  // Destructor

  ~LtgSpdbBuffer(void);
  
  // Adds the given strike to the strike buffer.

  void addStrike(LTG_strike_t *strike);
  void addStrike(LTG_extended_t *strike);
  
  // Writes the current SPDB lightning buffer to the database.
  // Returns 0 on success, -1 on failure

  int writeToDatabase(char *database_url,
		      int expire_secs,
		      int check_old_data);
  
  // Retrieves the number of strikes in the buffer

  inline int getNumStrikes() const
  {
    return _strikeBufferUsed;
  }
  

  // Detects if conflicting data types have been used.

  inline bool didTypesConflict() const
  {
    return _conflictingTypes;
  }

  // Clear the buffer.

  void clear(void);
  
 private:
  
  // Debug flag

  bool _debugFlag;
  
  // Local buffers

  LTG_strike_t *_strikeBuffer;
  LTG_strike_t *_strikeBufferBE;

  LTG_extended_t *_strikeBufferExtended;
  LTG_extended_t *_strikeBufferBEextended;

  int _strikeBufferUsed;
  int _strikeBufferAlloc;
  int _dataType; // What data type we are using.
  bool _conflictingTypes;
  bool _putModeUnique;

  const static int LTG_DATA_TYPE_UNDEFINED = 0; // Don't know yet what type we are using.
  const static int LTG_DATA_TYPE_CLASSIC = 1;   // We are using the LTG_strike_t
  const static int LTG_DATA_TYPE_EXTENDED = 2;   // Using LTG_extended_t
  
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
