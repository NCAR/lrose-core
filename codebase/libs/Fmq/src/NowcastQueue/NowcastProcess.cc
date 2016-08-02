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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/03 18:08:49 $
//   $Id: NowcastProcess.cc,v 1.5 2016/03/03 18:08:49 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * NowcastProcess: Class containing the information uniquely identifying
 *                 a Nowcast process.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <Fmq/NowcastProcess.hh>
#include <iostream>
using namespace std;


/**********************************************************************
 * Constructor
 */

NowcastProcess::NowcastProcess(const string &process_name,
			       const string &process_instance,
			       const pid_t process_id) :
  _processName(process_name),
  _processInstance(process_instance),
  _processId(process_id)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

NowcastProcess::~NowcastProcess(void)
{
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 *              Friend Functions                                      *
 **********************************************************************/

ostream& operator<<(ostream &s, const NowcastProcess &process_info)
{

  // Following did not compile for g++3.2
  //    return
  //      s << process_info._processName << "::" <<
  //      process_info._processInstance << "(" <<
  //      process_info._processId << ")";

  string info = process_info._processName;
  info += "::";
  info += process_info._processInstance;
  info += "(";
  info += process_info._processId;
  info += ")";
  return s << info;

}
    


