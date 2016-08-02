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
//   $Id: Semaphore.cc,v 1.5 2016/03/03 18:00:26 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Semaphore.cc
 *
 * C++ classes for using semaphores
 *
 * Nancy Rehak, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * April 1995
 *
 */


#include <iostream>

#include <toolsa/Semaphore.hh>
#include <toolsa/umisc.h>
using namespace std;

/*
 * General Semaphore class, member functions.
 */

Semaphore::Semaphore(int key)
{
  _key = key;
  _set_by_me = FALSE;
}


Semaphore:: ~Semaphore(void)
{
  // Empty
}

error_codes Semaphore::set(void)
{
  // See if we have already set the semaphore ourselves

  if (_set_by_me)
    return SEM_SUCCESS;

  // make sure the semaphore isn't already set.

  if (usem_test(_id, 0) != 0)
  {
    cerr << "Semaphore::set : ";
    cerr << "Cannot test semaphore, key = ";
    cerr << _key << endl;
    return SEM_SYSTEM_ERROR;
  }

  // set the semaphore

  if (usem_set(_id, 0) != 0)
  {
    cerr << "Semaphore::set : ";
    cerr << "Cannot set semaphore, key = ";
    cerr << _key << endl;
    return SEM_SYSTEM_ERROR;
  }

  // set the "sem set by me" flag

  _set_by_me = TRUE;

  return SEM_SUCCESS;
}


error_codes Semaphore::set_override(int secs)
{
  // make sure the semaphore isn't already set.

  for (int i = 0; i < secs; i++)
  {
    if (_set_by_me ||
	usem_check(_id, 0) == 0)
      break;

    sleep(1);
  } /* endfor - i */

  // set the semaphore

  if (usem_set(_id, 0) != 0)
  {
    cerr << "Semaphore::set : ";
    cerr << "Cannot set semaphore, key = ";
    cerr << _key << endl;

    _set_by_me = FALSE;

    return SEM_SYSTEM_ERROR;
  }

  // set the "sem set by me" flag

  _set_by_me = TRUE;

  return SEM_SUCCESS;
}


error_codes Semaphore::clear(void)
{
  // make sure we are the ones who set the semaphore

  if (!_set_by_me)
  {
    cerr << "Semaphore::clear : ";
    cerr << "Cannot clear semaphore, key = ";
    cerr << _key << ", set by another process" << endl;
    return SEM_NOT_OWNER;
  }

  // clear the "sem set by me" flag

  _set_by_me = FALSE;

  // clear the semaphore

  if (usem_clear(_id, 0) != 0)
  {
    cerr << "Semaphore::clear : ";
    cerr << "Cannot clear semaphore, key = ";
    cerr << _key << endl;
    return SEM_SYSTEM_ERROR;
  }

  return SEM_SUCCESS;
}

error_codes Semaphore::clear_override(int secs)
{
  // make sure we are the ones who set the semaphore

  if (!_set_by_me)
  {
    for (int i = 0; i < secs; i++)
    {
      if (usem_check(_id, 0) == 0)
        return SEM_SUCCESS;

      sleep(1);
    } /* endfor - i */
  }

  // clear the "sem set by me" flag

  _set_by_me = FALSE;

  // clear the semaphore

  if (usem_clear(_id, 0) != 0)
  {
    cerr << "Semaphore::clear : ";
    cerr << "Cannot clear semaphore, key = ";
    cerr << _key << endl;
    return SEM_SYSTEM_ERROR;
  }

  return SEM_SUCCESS;
}

int Semaphore::check(void)
{
  return usem_check(_id, 0);
}



/*
 * Derived MasterSem class, member functions.
 */

MasterSem::MasterSem(int key) : Semaphore(key)
{
  // create the semaphore

  if ((_id = usem_create(_key, 1, 0666)) < 0)
  {
    cerr << "MasterSem::MasterSem : ";
    cerr << "Cannot create semaphore set, key = ";
    cerr << _key << endl;
    exit(-1);
  }
  
  // clear the semaphore

  if (usem_clear(_id, 0) != 0)
  {
    cerr << "MasterSem::MasterSem :";
    cerr << "Cannot clear semaphore set, key = ";
    cerr << _key << endl;
    exit(-1);
  }
}

MasterSem::~MasterSem(void)
{
  // delete the semaphore

  if (usem_remove(_key) != 0)
  {
    cerr << "MasterSem::~MasterSem : ";
    cerr << "Cannot delete semaphore set, key = ";
    cerr << _key << endl;
  }
}


/*
 * Derived SlaveSem class, member functions.
 */

SlaveSem::SlaveSem(int key) : Semaphore(key)
{
  // get the semaphore set

  if ((_id = usem_get(_key, 1)) < 0)
  {
    cerr << "SlaveSem::SlaveSem : ";
    cerr << "Cannot get semaphore set, key = ";
    cerr << _key << endl;
    exit(-1);
  }
}


SlaveSem::~SlaveSem(void)
{
  // Empty
}
