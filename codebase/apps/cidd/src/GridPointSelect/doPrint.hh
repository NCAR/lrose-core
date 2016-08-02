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
// doPrint.hh
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// December 2003
//

#ifndef doPrint_H
#define doPrint_H

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxProj.hh>

#include "Params.hh"
using namespace std;

class doPrint {
  
public:

  // constructor
  doPrint (Params *TDRP_Params, int refNum);

  // destructor
  ~doPrint();

  // Do a printout of data at the time/location specified.

  void printData(time_t dataTime,
		 double lat,
		 double lon);
 
protected:
private:

  // Update data.

  void _dataUpdate(time_t dataTime);

  // data members

  bool _dataOK;          // Did the data reads go OK?
  bool _firstTime;       // Is it the first time we have been called?
  Params *_params;       // Our own pointer to the parameters.
  int _refNum;           // Which URL we are looking at.
  MdvxProj *_Proj;       // The projection object.
  DsMdvx   _mdvxObject;  // The data object.
  time_t   _lastTime;    // The data request time.
  time_t   _actualDataTime; // The time of the actual data.
  fl32     *_dataPointer;   // Pointer to the data.
  Mdvx::field_header_t *_Fhdr;
  Mdvx::vlevel_header_t *_Vhdr;

};

#endif
