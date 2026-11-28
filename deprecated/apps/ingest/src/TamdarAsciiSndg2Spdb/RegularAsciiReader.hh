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
 *   $Date: 2016/03/07 01:23:06 $
 *   $Id: RegularAsciiReader.hh,v 1.2 2016/03/07 01:23:06 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * RegularAsciiReader: Class that reads Sndg information from a regular
 *                     format ASCII file.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2005
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef RegularAsciiReader_H
#define RegularAsciiReader_H

#include "AsciiReader.hh"

using namespace std;


class RegularAsciiReader : public AsciiReader
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  RegularAsciiReader(const bool debug_flag);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~RegularAsciiReader();


  /*********************************************************************
   * openFile() - Open the input file.
   *
   * Returns true on success, false on failure.
   */

  virtual bool openFile(const string &ascii_filepath);


  /*********************************************************************
   * getNextSndg() - Read the next sounding from the input file.
   *
   * Returns TRUE if a sounding was read, FALSE if there are no more
   * soundings in the file.
   */

  virtual bool getNextSndg(Sndg &sounding);


protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  static const double MISSING_DATA_VALUE;
  static const int INPUT_LINE_LEN;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  char *_nextInputLine;
  
};

#endif
