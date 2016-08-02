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
 * GaiLtgFile.hh : header file for the GaiLtgFile program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2002
 *
 * Gary Blackburn
 *
 ************************************************************************/

#ifndef GaiLtgFile_HH
#define GaiLtgFile_HH

/*
 **************************** includes **********************************
 */


#include <cstdio>
#include <vector>


#include <rapformats/gailtg.h>
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

class GaiLtgFile
{
 public:

  // Constructor

  GaiLtgFile(bool debug_flag, bool old_data_flag);
  
  // Destructor

  ~GaiLtgFile(void);
  
  // Load the given data file into the object.  Wait the given number
  // of seconds before loading the file to make sure the creating
  // process has time to finish writing the file.
  //
  // Returns true if the load was successful, false otherwise.

  bool loadFile(const char *filename, int processing_delay = 0);
  
  
  // Close the current GAI lightning file

  void close(void)
  {
    // do nothing
  }

  GAILTG_strike_t *getFirstStrike();
  GAILTG_strike_t *getNextStrike();

 private:
  
  // flag to read old GAI data

  bool _fiveFields;

  // Debug flag

  bool _debugFlag;
  
  char **_tokens;
  static const int MAX_TOKENS;
  static const int MAX_TOKEN_LEN;

  // Input data

  vector <GAILTG_strike_t *>::iterator _strikeIterator;
  vector <GAILTG_strike_t *> _strikeBuffer;
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("GaiLtgFile");
  }
  
};


#endif
