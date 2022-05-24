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

Header: TempData.hh

Author: Dave Albo

Date:   Thu Jan 26 16:43:33 2006

Description: Holds copy of one data grid that is needed later.

*************************************************************************/

# ifndef    TempData_HH
# define    TempData_HH

/* System include files / Local include files */
#include <Mdv/DsMdvx.hh>
#include "Data.hh"
using namespace std;

/* Class definitions */
class TempData
{
public:

  // empty.
  TempData();
  
  // copy constructor
  TempData(const TempData &);
  
  // destructor
  virtual ~TempData();

  // operator overloading.
  void operator=(const TempData &);
  bool operator==(const TempData &) const;

  void print(void) const;

  ////////////////////////////////////////////////////////////////
  /////////////////////// simple filters /////////////////////////
  ////////////////////////////////////////////////////////////////

  // clear out memory allocation internally.
  void clear(void);

  // store inputs as new state.
  void store(Data &D);

  // extract into D, overwrite the data that was in D.
  bool extract(Data &D);

  // return pointer to the data, and the data missing/bad values,
  // plus number of data values.
  fl32 *getVals(fl32 &missing, fl32 &bad, int &num) const;

  // change the bad/missing to input, along with the data itself.
  void putVals(fl32 missing, fl32 bad);

protected:
private:  

  ////////////////////////////////////////////////////////////////
  /////////////////////// private members ////////////////////////
  ////////////////////////////////////////////////////////////////

  fl32 *_data;   // pointer to a COPY of the data..owned by this object.
  int _ndata;    // size of data
  fl32 _missing; // data missing value.
  fl32 _bad;     // data bad value.
  int _nx, _ny, _nz; // dimensions of data.
};

# endif     /* TempData_HH */
