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
 *   $Id: ShmemSem.hh,v 1.5 2016/03/03 19:26:12 dixon Exp $
 *   $Revision: 1.5 $
 *   $State: Exp $
 *
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/******************************************************************
 * ShmemSem.h - include file for the ShmemSem class.
 *
 * The ShmemSem class is a simple combination of the Semaphore and
 * Shmem classes.  This class is useful in that you create a single
 * object that represents both the semaphore and the shared memory.
 * The constructor of this class creates both the semaphore set and the
 * shared memory (using the same key for each) for you.  You then use
 * the set()/clear() functions of the Semaphore class to set and clear
 * the semaphore when you access the shared memory.  The shared
 * memory buffer is accessed through the buffer pointer inherited from
 * the Shmem class.
 *
 * In this class, I assume that you care about which process actually
 * creates the shared memory segment, thus I have a MasterShmemSem
 * object (used by the creator) and a SlaveShmemSem object (used by
 * the accessor).  Both processes can update the shared memory.
 *
 * Nancy Rehak
 *
 * May 1995
 *
 ******************************************************************/

#ifndef SHMEMSEM_H
#define SHMEMSEM_H


#include <toolsa/Semaphore.hh>
#include <toolsa/Shmem.hh>
using namespace std;


class MasterShmemSem : public MasterSem, public MasterShmem
{
 public:
  
  MasterShmemSem(int key, int size, int prot);
  ~MasterShmemSem(void);
};


class SlaveShmemSem : public SlaveSem, public SlaveShmem
{
 public:
  
  SlaveShmemSem(int key, int size, int prot);
  ~SlaveShmemSem(void);
};


#endif
