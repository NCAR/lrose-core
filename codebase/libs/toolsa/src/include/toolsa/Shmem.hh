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
 *   $Id: Shmem.hh,v 1.5 2016/03/03 19:26:12 dixon Exp $
 *   $Revision: 1.5 $
 *   $State: Exp $
 *
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/******************************************************************
 * Shmem.h - include file for the Shmem class.
 *
 * The Shmem class handles accessing shared memory.  The Shmem
 * class is an abstract class that other classes should build on.
 * It does not create shared memory itself and so is probably not
 * useful by itself.
 *
 * In this class, I assume that you care about which process actually
 * creates the shared memory.  Thus, there is a MasterShmem subclass
 * (used by the creator) and a SlaveShmem subclass (used by the accessor).
 * Both processes can update the shared memory.
 *
 * Nancy Rehak
 * 
 * May 1995
 *
 ******************************************************************/

#ifndef SHMEM_H
#define SHMEM_H


#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
using namespace std;


class Shmem
{
 public:
  
  Shmem(key_t key, size_t size, int perm);
  ~Shmem(void);

  char *buffer;
  
 protected:
  
  key_t  _key;
  size_t _size;
  int    _perm;
};


class MasterShmem : public Shmem
{
 public:
  
  MasterShmem(key_t key, size_t size, int perm);
  ~MasterShmem(void);

};


class SlaveShmem : public Shmem
{
 public:
  
  SlaveShmem(key_t key, size_t size, int perm);
  ~SlaveShmem(void);
};


#endif
