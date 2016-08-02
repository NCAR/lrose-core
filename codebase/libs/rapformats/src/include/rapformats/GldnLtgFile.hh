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
 * GldnLtgFile.hh : header file for the GldnLtgFile program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2004
 *
 * Gary Blackburn/Kay Levesque
 *
 ************************************************************************/

#ifndef GldnLtgFile_HH
#define GldnLtgFile_HH

#include <cstdio>
#include <vector>

#include <rapformats/ltg.h>
using namespace std;

class GldnLtgFile
{
 public:

  // Constructor

  GldnLtgFile(bool debug_flag);
  
  // Destructor

  ~GldnLtgFile(void);
  
  // Load the given data file into the object.  Wait the given number
  // of seconds before loading the file to make sure the creating
  // process has time to finish writing the file.
  //
  // Returns true if the load was successful, false otherwise.

  bool loadFile(const char *filename, int processing_delay = 0);
  
  
  // Close the current GLDN lightning file

  void close(void)
  {
    // do nothing
  }

  LTG_strike_t *getFirstStrike();
  LTG_strike_t *getNextStrike();

 private:
  
  // Debug flag

  typedef enum
  {
    GLDNLTG_GROUND_STROKE = 0, GLDNLTG_CLOUD_STROKE = 1
  }ltg_type_t;

  bool _debugFlag;
  
  char **_tokens;
  static const int MAX_TOKENS;
  static const int MAX_TOKEN_LEN;

  // Input data

  vector <LTG_strike_t *>::iterator _strikeIterator;
  vector <LTG_strike_t *> _strikeBuffer;
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("GldnLtgFile");
  }
  
};


#endif
