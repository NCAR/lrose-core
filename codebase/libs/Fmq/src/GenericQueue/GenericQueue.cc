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
//   $Id: GenericQueue.cc,v 1.4 2016/03/03 18:08:49 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * GenericQueue.cc: class implementing a generic message queue.  This
 *                  queue handles message common to several applications
 *                  like the trigger message.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2000
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <dataport/bigend.h>
#include <Fmq/GenericQueue.hh>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/DateTime.hh>
#include <toolsa/MsgLog.hh>
using namespace std;


/**********************************************************************
 * Constructors
 */

GenericQueue::GenericQueue() :
  DsFmq()
{
  // Do nothing
}

/**********************************************************************
 * Destructor
 */

GenericQueue::~GenericQueue(void)
{
  // Do nothing
}
  

/**********************************************************************
 * fireTrigger() - Fire the trigger message.
 *
 * Returns 0 on success, -1 on error.
 */

int GenericQueue::fireTrigger(const string &says_who,
			      time_t trigger_time, time_t forecast_time)
{
  trigger_t trigger;

  // Setup the trigger request
  // Do the necessary byte swapping

  trigger.trigger_time = BE_from_si32((si32)trigger_time);
  trigger.forecast_time = BE_from_si32((si32)forecast_time);
  
  STRncopy( trigger.says_who, says_who.c_str(), (int)NAME_LEN );

  return writeMsg(TRIGGER_MSG, 0, (void*)(&trigger), sizeof(trigger));
}


/**********************************************************************
 * nextTrigger() - Read the next trigger message.
 *
 * Returns 0 on success, -1 on error.
 */

int GenericQueue::nextTrigger(string &says_who,
			      time_t *trigger_time, time_t *forecast_time)
{
  bool gotMsg;
  int status;                                            
  trigger_t trigger;
   
  status = readMsg( &gotMsg, TRIGGER_MSG );
  if (status == -1 || !gotMsg || getMsgLen() == 0)
  {
    return( -1 );                                               
  }
  else
  {
    // Decode the trigger message
    // Do the necessary byte swapping

    trigger = *((trigger_t*)(getMsg()));

    *trigger_time = (time_t)BE_to_si32(trigger.trigger_time);
    *forecast_time = (time_t)BE_to_si32(trigger.forecast_time);
    says_who = trigger.says_who;

    return 0;  
  }
}
