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
/************************************************************************

Header: TempDataState.hh

Author: Dave Albo

Date:   Fri Jan 27 12:12:54 2006

Description: Temporary data storage class, all data that needs to be kept around.

*************************************************************************/

# ifndef    TempDataState_HH
# define    TempDataState_HH

/* System include files / Local include files */
#include <map>
#include <string>
#include <cstdio>
#include "TempData.hh"
#include "Data.hh"
#include "Params.hh"
using namespace std;

/* Class definition */
class TempDataState
{
public:

  // default constructor
  TempDataState();
  
  // destructor
  virtual ~TempDataState();

  void print(void) const;

  // initialize so all data that needs to be kept can be stored.
  // nothing gets stored.
  void init(const Params &P);

  // clear out all data allocations so that all data storage has been freed.
  void clear(void);

  //store input data if it is data that needs to be kept.
  void store_if_needed(string &name, Data &D);

  //extract from name'th internal data into D, overwrite the data in D.
  bool extract(string &name, Data &D);

  // true if name is one of the internal names.
  bool has_name(string &name) const;

  // return pointer to data, missing, bad, and num for name.
  // return NULL if problems.
  fl32 *getVals(string &name, fl32 &missing, fl32 &bad, int &num);

  // change missing/bad values to inputs for name.
  void putVals(string &name, fl32 missing, fl32 bad);

protected:
private:  

  ////////////////////////////////////////////////////////////////
  /////////////////////// private members ////////////////////////
  ////////////////////////////////////////////////////////////////

  // the string is the name of the field, the TempData is the content
  map<string, TempData> _state;
  
  ////////////////////////////////////////////////////////////////
  /////////////////////// private methods ////////////////////////
  ////////////////////////////////////////////////////////////////
  
  void _add(const char *name);

};

# endif     /* TempDataState_HH */
