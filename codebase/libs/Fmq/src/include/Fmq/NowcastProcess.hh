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
 *   $Date: 2016/03/03 18:56:47 $
 *   $Id: NowcastProcess.hh,v 1.5 2016/03/03 18:56:47 dixon Exp $
 *   $Revision: 1.5 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * NowcastProcess: Class containing the information uniquely identifying
 *                 a Nowcast process.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2001
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef NowcastProcess_HH
#define NowcastProcess_HH

/*
 **************************** includes **********************************
 */

#include <string>
#include <sys/types.h>
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

class NowcastProcess
{
 public:

  ////////////////////////////////
  // Constructors / Destructors //
  ////////////////////////////////

  /**********************************************************************
   * Constructor
   */

  NowcastProcess(const string &process_name = "",
		 const string &process_instance = "",
		 const pid_t process_id = 0);
  

  /**********************************************************************
   * Destructor
   */

  ~NowcastProcess(void);
  

  //////////////////////
  // Accessor methods //
  //////////////////////

  /**********************************************************************
   * empty() - Check to see if the process information is empty.
   */

  inline bool empty(void)
  {
    return _processName.empty();
  }
  

  /**********************************************************************
   * setProcessName() - Set the name for the process.  This usually is
   *                    the executable name.
   */

  inline void setProcessName(const string &process_name)
  {
    _processName = process_name;
  }
  

  /**********************************************************************
   * getProcessName() - Retrieve the name for the process.
   */

  inline string getProcessName(void)
  {
    return _processName;
  }
  

  /**********************************************************************
   * setProcessInstance() - Set the instance for the process.  This
   *                        usually is the instance used for the process
   *                        mapper.
   */

  inline void setProcessInstance(const string &process_instance)
  {
    _processInstance = process_instance;
  }
  

  /**********************************************************************
   * getProcessInstance() - Retrieve the instance for the process.
   */

  inline string getProcessInstance(void)
  {
    return _processInstance;
  }
  

  /**********************************************************************
   * setPID() - Set the PID for the process.
   */

  inline void setPID(const pid_t pid)
  {
    _processId = pid;
  }
  

  /**********************************************************************
   * getPID() - Retrieve the PID for the process.
   */

  inline pid_t getPID(void)
  {
    return _processId;
  }
  

  //////////////////////////
  // Input/Output methods //
  //////////////////////////

  /**********************************************************************
   * toString() - Convert the object information to a string for printing.
   */

  inline string toString(void) const
  {
    return _processName + "::" + _processInstance;
  }
  

  ///////////////
  // Operators //
  ///////////////

  bool operator==(const NowcastProcess &other) const
  {
    return _processName == other._processName &&
      _processInstance == other._processInstance;
  }
  

  bool operator!=(const NowcastProcess &other) const
  {
    return _processName != other._processName ||
      _processInstance != other._processInstance;
  }
  

  bool operator<(const NowcastProcess &other) const
  {
    if (_processName != other._processName)
      return(_processName < other._processName ? true : false);
    
    return(_processInstance < other._processInstance ? true : false);
  }
  

  bool operator<=(const NowcastProcess &other) const
  {
    if (_processName != other._processName)
      return(_processName < other._processName ? true : false);
    
    return(_processInstance <= other._processInstance ? true : false);
  }
  

  bool operator>(const NowcastProcess &other) const
  {
    if (_processName != other._processName)
      return(_processName > other._processName ? true : false);
    
    return(_processInstance > other._processInstance ? true : false);
  }
  

  bool operator>=(const NowcastProcess &other) const
  {
    if (_processName != other._processName)
      return(_processName > other._processName ? true : false);
    
    return(_processInstance >= other._processInstance ? true : false);
  }
  

  friend ostream& operator<<(ostream &s, const NowcastProcess &process_info);


 protected:

  string _processName;
  string _processInstance;
  pid_t _processId;
  

 private:

  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("NowcastProcess");
  }
  
};



///////////////////////////////
// Class for STL comparisons //
///////////////////////////////

class NowcastProcessCompare
{
public:

  bool operator()(const NowcastProcess &proc1,
		  const NowcastProcess &proc2) const
  {
    return proc1 < proc2;
  }
  
};


#endif
