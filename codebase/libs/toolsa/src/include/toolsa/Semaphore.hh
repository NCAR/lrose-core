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

/* SCCS info
 *   %W% %D% %T%
 *   %F% %E% %U%
 *
 * RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/03 19:26:12 $
 *   $Id: Semaphore.hh,v 1.5 2016/03/03 19:26:12 dixon Exp $
 *   $Revision: 1.5 $
 *   $State: Exp $
 *
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/******************************************************************
 * Semaphore.h - include file for the Semaphore class.
 *
 * The Semaphore class handles accessing semaphore sets.  It assumes
 * that you only care about one bit in the semaphore.  This class is
 * an abstract class that other classes should build on.  It does
 * not create semaphore sets itself and so is probably not useful
 * by itself.
 *
 * In this class, I assume that you care about which process actually
 * creates the semaphore set.  Thus, there is a MasterSem subclass
 * (used by the creator) and a SlaveSem subclass (used by the
 * accessor).  Both processes can update the semaphore.
 *
 * Nancy Rehak
 *
 * May 1995
 *
 ******************************************************************/

#ifndef SEMAPHORE_H
#define SEMAPHORE_H


#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
using namespace std;


enum error_codes { SEM_SUCCESS,
                   SEM_SYSTEM_ERROR,
                   SEM_NOT_OWNER };

class Semaphore
{
 public:
  
  Semaphore(int sem_key);
  ~Semaphore(void);
  
  error_codes set(void);                // Set the semaphore.  If the
                                        // semaphore was already set by
                                        // this object, returns success.
                                        // If the semaphore is set by
                                        // another object, blocks until
                                        // the semaphore is cleared.

  error_codes set_override(int secs);   // Works the same as set(), but
                                        // if the semaphore is set by
                                        // another object, waits up to the
                                        // given number of seconds (checking
                                        // every second) for the semaphore
                                        // to clear.  If the semaphore is
                                        // not cleared, overrides the other
                                        // process (assuming it has died)
                                        // and sets the semaphore itself.

  error_codes clear(void);              // Clears the semaphore.  If the
                                        // semaphore was set by another
                                        // process, leaves the semaphore
                                        // set and returns SEM_NOT_OWNER.

  error_codes clear_override(int secs); // Works the same as clear(), but
                                        // if the semaphore is set by another
                                        // object waits up to the given
                                        // number of seconds for the semaphore
                                        // to clear.  If the semaphoer is
                                        // not cleared, overrides the other
                                        // process (assuming it has died)
                                        // and clears the semaphore itself.

  int check(void);                      // Checks the current value of the
                                        // semaphore.
 protected:
  
  key_t _key;
  int   _id;
  int   _set_by_me;
};


class MasterSem : public Semaphore
{
 public:
  
  MasterSem(int sem_key);
  ~MasterSem(void);
};


class SlaveSem : public Semaphore
{
 public:
  
  SlaveSem(int sem_key);
  ~SlaveSem(void);
};


#endif
