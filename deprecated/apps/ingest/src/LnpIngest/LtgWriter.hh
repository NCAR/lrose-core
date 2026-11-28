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
 *   $Date: 2016/03/07 01:23:02 $
 *   $Id: LtgWriter.hh,v 1.3 2016/03/07 01:23:02 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * LtgWriter: Class for handling writing lightning strike data to the
 *            output file.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2006
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef LtgWriter_HH
#define LtgWriter_HH

#include <didss/LdataInfo.hh>
#include <toolsa/DateTime.hh>

#include "LtgStrike.hh"

using namespace std;


class LtgWriter
{
 public:

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Constructors
   */

  LtgWriter();
  

  /*********************************************************************
   * Destructor
   */

  virtual ~LtgWriter(void);
  

  /*********************************************************************
   * init() - Initialize the object.  This must be called before any other
   *          methods.
   *
   * Returns true on success, false on failure.
   */

  bool init(const string &output_dir,
	    const int seconds_per_file);
  

  //////////////////////////
  // Input/Output methods //
  //////////////////////////

  /*********************************************************************
   * writeStrike() - Write the given strike to the appropriate output
   *                  file.
   *
   * Returns true on success, false on failure.
   */

  bool writeStrike(const LtgStrike &strike);
  

 private:

  ///////////////////////
  // Private constants //
  ///////////////////////

  static const string FILE_EXT;


  /////////////////////
  // Private members //
  /////////////////////

  string _outputDir;
  int _secondsPerFile;
  LdataInfo _ldataInfo;
  
  FILE *_currFile;
  string _currFilename;
  DateTime _currFileTime;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * _closeFile() - Close the current output file.
   */

  inline void _closeFile()
  {
    if (_currFile != 0)
    {
      fclose(_currFile);

      _ldataInfo.setRelDataPath(_currFilename.c_str());
      _ldataInfo.write(_currFileTime.utime());
      
      _currFile = 0;
      _currFilename = "";
      _currFileTime = DateTime::NEVER;
    }
  }


  /*********************************************************************
   * _getFirstFileTime() - This is called when we don't have a file currently
   *                       open.  So, we need to figure out the correct file
   *                       time to use by starting at the beginning of the
   *                       current hour and iterating through the specified
   *                       file duration.
   *
   * Returns the correct file time to use.
   */

  DateTime _getFirstFileTime(const DateTime &strike_time) const;
  

  /*********************************************************************
   * _getNextFileTime() - This is called when we have a file currently open,
   *                      but the current strike needs to go in a new file.
   *                      In this case we start at the current file time and
   *                      iterate through the specified file duration until
   *                      we get the appropriate file for the new strike.
   *
   * Returns the correct file time to use.
   */

  DateTime _getNextFileTime(const DateTime &strike_time) const;
  

  /*********************************************************************
   * _openFile() - Make sure that the currently open file is the appropriate
   *               one for the given time.
   *
   * Returns true on success, false on failure.
   */

  bool _openFile(const DateTime &strike_time);
  

};


#endif
