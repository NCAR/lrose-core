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

// SCCS info
//   %W% %D% %T%
//   %F% %E% %U%
//
// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/03 18:00:26 $
//   $Id: Shmem.cc,v 1.5 2016/03/03 18:00:26 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Shmem.cc
 *
 * C++ classes for using shared memory
 *
 * Nancy Rehak, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * April 1995
 *
 */


#include <cerrno>
#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <toolsa/Shmem.hh>
#include <toolsa/umisc.h>
using namespace std;

/*
 * General Shmem class, member functions.
 */

Shmem::Shmem(key_t key, size_t size, int perm)
{
  _key = key;
  _size = size;
  _perm = perm;
}


Shmem:: ~Shmem(void)
{
  // Empty
}


/*
 * Derived MasterShmem class, member functions.
 */

MasterShmem::MasterShmem(key_t key, size_t size, int perm) :
Shmem(key, size, perm)
{
  // create the shared memory

  if ((this->buffer = (char *)ushm_create(_key, 
				    _size,
					  _perm)) == NULL)
  {
    cerr << "MasterShmem::MasterShmem : ";
    cerr << "Cannot create shared memory, key = ";
    cerr << _key << ", size = " << _size << endl;
    exit(-1);
  }
  
  // clear the shared memory

  memset((void *)this->buffer, (int)0, _size);

}

MasterShmem::~MasterShmem(void)
{
  // delete the shmem

  if (ushm_remove(_key) != 0)
  {
    cerr << "MasterShmem::~MasterShmem : ";
    cerr << "Cannot delete shared memory, key = ";
    cerr << _key << endl;
  }
}


/*
 * Derived SlaveShmem class, member functions.
 */

SlaveShmem::SlaveShmem(key_t key, size_t size, int perm) :
Shmem(key, size, perm)
{
  // attach to the shared memory

  if ((this->buffer = (char *)ushm_get(_key, 
				 _size)) == NULL)
  {
    cerr << "SlaveShmem::SlaveShmem : ";
    cerr << "Cannot attach to shared memory, key = ";
    cerr << _key << ", size = " << _size << endl;
    exit(-1);
  }
  
  // make sure the shared memory is of the expected size

//  struct shmid_ds shmem_buffer;
//  if (shmctl(_id, IPC_STAT, &shmem_buffer) < 0)
//  {
//    cerr << "SlaveShmem::SlaveShmem : ";
//    cerr << "Cannot status shared memory, key = ";
//    cerr << _key << ", size = " << _size << endl;
//    cerr << "errno = " << errno << endl;
//    exit(-1);
//  }
//
//  if (shmem_buffer.shm_segsz != _size)
//  {
//    cerr << "SlaveShmem::SlaveShmem : ";
//    cerr << "Created shared memory not of desired size: " << endl;
//    cerr << "   shared memory size = " << shmem_buffer.shm_segsz;
//    cerr << ", desired size = " << _size << endl;
//  }

}


SlaveShmem::~SlaveShmem(void)
{
  // Empty
}
