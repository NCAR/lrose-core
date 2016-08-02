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
//   $Id: ShmemSem.cc,v 1.5 2016/03/03 18:00:26 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * ShmemSem.cc
 *
 * C++ classes for using shared memory with associated semaphores
 *
 * Nancy Rehak, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * April 1995
 *
 */


#include <iostream>

#include <toolsa/ShmemSem.hh>
#include <toolsa/umisc.h>
using namespace std;

/*
 * Master ShmemSem class, member functions.
 */

MasterShmemSem::MasterShmemSem(int key, int size, int prot) :
MasterSem(key), MasterShmem(key, size, prot)
{
  // Empty
}


MasterShmemSem:: ~MasterShmemSem(void)
{
  // Empty
}


/*
 * Slave ShmemSem class, member functions.
 */

SlaveShmemSem::SlaveShmemSem(int key, int size, int prot) :
SlaveSem(key), SlaveShmem(key, size, prot)
{
  // Empty
}


SlaveShmemSem:: ~SlaveShmemSem(void)
{
  // Empty
}


