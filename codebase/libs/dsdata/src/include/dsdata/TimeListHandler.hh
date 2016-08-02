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
 *   $Date: 2016/03/03 18:06:34 $
 *   $Id: TimeListHandler.hh,v 1.4 2016/03/03 18:06:34 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * TimeListHandler: Base class for handles the processing of a time list.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2002
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef TimeListHandler_HH
#define TimeListHandler_HH

/*
 **************************** includes **********************************
 */

#include <string>
#include <ctime>
using namespace std;

/*
 ************************* class definitions ****************************
 */

class TimeListHandler
{

public:

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /**********************************************************************
   * Constructors
   */

  TimeListHandler(const string &url,
		  const time_t start_time,
		  const time_t end_time);


  /**********************************************************************
   * Destructor
   */

  virtual ~TimeListHandler();


  ////////////////////
  // Access methods //
  ////////////////////

  /**********************************************************************
   * next() - Get the next trigger and set the triggerInfo accordingly
   *
   * Returns the next data time upon success, -1 upon failure or if there
   * are no more data times to process.
   */

  virtual time_t next() = 0;
  

  /**********************************************************************
   * endOfData() - Check whether we are at the end of the data.
   */

  virtual bool endOfData() const = 0;
  

  /**********************************************************************
   * reset() - Reset to start of data list
   */

  virtual void reset() = 0;
  

  ///////////////////
  // Error methods //
  ///////////////////

  /**********************************************************************
   * getErrStr() - Return the error string.
   */

  const string& getErrStr() const
  {
    return getErrString();
  }

  const string& getErrString() const
  {
    return _errStr;
  }


protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  string _errStr;

  string _url;

  time_t _startTime;
  time_t _endTime;
  
  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**********************************************************************
   * _clearErrStr() - Set _errStr to the empty string.
   */

  void _clearErrStr()
  {
    _errStr = "";
  }


};

#endif /* TimeListHandler_HH */


