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
 *   $Id: GenericQueue.hh,v 1.4 2016/03/03 18:56:47 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * GenericQueue.hh: class implementing a generic message queue.  This
 *                  queue handles message common to several applications
 *                  like the trigger message.
 *
 * The code for this class was taken from NowcastQueue.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2000
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef _GENERIC_QUEUE_INC_
#define _GENERIC_QUEUE_INC_

/*
 **************************** includes **********************************
 */

#include <string>
#include <unistd.h>

#include <Fmq/DsFmq.hh>
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


class GenericQueue : public DsFmq
{
public:

  //////////////////////
  // Public constants //
  //////////////////////

  static const int NAME_LEN = 128;


  //////////////////
  // Public types //
  //////////////////

  enum msgType { TRIGGER_MSG = 100000 };


  ////////////////////////////////
  // Constructors & destructors //
  ////////////////////////////////

  // Constructors

   GenericQueue(void);

  // Destructor

  virtual ~GenericQueue(void);
  

  ///////////////////////////////////////////
  // Methods for handling trigger messages //
  //                                       //
  // Trigger messages are used when one    //
  // process needs to trigger an action in //
  // another process.                      //
  ///////////////////////////////////////////

  // Read the next trigger message.
  //
  // Returns 0 on success, -1 on error.

  int nextTrigger(string &says_who,
		  time_t *trigger_time, time_t *forecast_time);

  // Fire the trigger message.
  //
  // Returns 0 on success, -1 on error.

  int fireTrigger(const string &says_who,
		  time_t trigger_time, time_t forecast_time = -1);


private:

  ////////////////////////
  // Message structures //
  ////////////////////////

  typedef struct
  {
    si32  trigger_time;
    si32  forecast_time;
    char  says_who[NAME_LEN];
  } trigger_t;

};

#endif
