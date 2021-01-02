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
//////////////////////////////////////////////////////////
// RandomList.cc
//
// Generates lists with random seed-no_seed splits
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1997
//
//////////////////////////////////////////////////////////

#include "RandomList.hh"
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <cstdlib>
#include <math.h>
#include <ctime>
using namespace std;

//////////////
// Constructor

RandomList::RandomList (const char *prog_name,
			int debug,
			int n_random_list,
			int max_split)

{

  // initialize
  
  _progName = STRdup(prog_name);
  _debug = debug;
  _nRandomList = n_random_list;
  _maxSplit = max_split;

  // create list array

  _list = (int *) umalloc(_nRandomList * sizeof(int));

  // initialize random number generator

  srandom(time(NULL));

}

/////////////
// Destructor

RandomList::~RandomList()

{

  // free up memory

  STRfree(_progName);
  ufree(_list);

}

////////////////////////////////////////
// generate()
//
// Generate a new list.
//
// returns pointer to list
//
////////////////////////////////////////

#ifdef SUNOS4
#define RAND_MAX 0x7fffffff
#endif

int *RandomList::generate()
  
{

  int split = _maxSplit + 1;
  long half = RAND_MAX / 2;

  while (split > _maxSplit) {

    int nyes = 0;
    
    int *l = _list;

    for (int i = 0; i < _nRandomList; i++, l++) {

      long r = random();
      
      if (r < half) {
	*l = 0;
      } else {
	*l = 1;
	nyes++;
      }

    } // i

    int nno = _nRandomList - nyes;
    
    split = abs(nyes - nno);

  } // while

  return (_list);

}

