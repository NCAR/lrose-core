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
 *   $Date: 2016/03/07 01:39:56 $
 *   $Id: AsciiReader.hh,v 1.2 2016/03/07 01:39:56 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * AsciiReader: Base class for classes that read Sndg information from
 *              ASCII files.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2005
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef AsciiReader_H
#define AsciiReader_H

#include <cstdio>

#include <rapformats/Sndg.hh>
#include <toolsa/Path.hh>

using namespace std;


class AsciiReader
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  AsciiReader(const bool debug_flag);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~AsciiReader();


  /*********************************************************************
   * openFile() - Open the input file.
   *
   * Returns true on success, false on failure.
   */

  virtual inline bool openFile(const string &ascii_filepath)
  {
    static const string method_name = "AsciiReader::openFile()";
    
    // Close the last file if that wasn't already done

    if (_asciiFile != 0)
      closeFile();
    
    // Now open the new file

    if ((_asciiFile = fopen(ascii_filepath.c_str(), "r")) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error opening input ASCII file: " << ascii_filepath << endl;
      
      return false;
    }
    
    return true;
  }


  /*********************************************************************
   * getNextSndg() - Read the next sounding from the input file.
   *
   * Returns TRUE if a sounding was read, FALSE if there are no more
   * soundings in the file.
   */

  virtual bool getNextSndg(Sndg &sounding) = 0;


  /*********************************************************************
   * closeFile() - Close the output file.
   */

  virtual inline void closeFile()
  {
    if (_asciiFile != 0)
      fclose(_asciiFile);
    _asciiFile = 0;
  }


protected:

  bool _debug;
  
  FILE *_asciiFile;

  
};

#endif
