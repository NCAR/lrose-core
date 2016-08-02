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


#include "Process.hh"

#include <iostream>
#include <Mdv/MdvxField.hh>
#include <math.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>
#include <Mdv/MdvxProj.hh>
using namespace std;

//
// Constructor
//
Process::Process(){
  isFirst = true;
  return;
}

////////////////////////////////////////////////////
//
// Main method - process data at a given time.
//
int Process::Derive(char *url, time_t T){


  //
  // Set up for the new data.
  //
  DsMdvx New;

  New.setReadTime(Mdvx::READ_FIRST_BEFORE, url, 0, T);
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  if (New.readVolume()){
    cerr << "Read failed at " << utimstr(T) << " from ";
    cerr << url  << endl;
    return -1;
  }     


  MdvxField *InField = New.getFieldByNum( 0 );
  if (InField == NULL) return 0;
  Mdvx::field_header_t InFhdr = InField->getFieldHeader();

  if (isFirst){
    isFirst = false;
    firstLat = InFhdr.proj_origin_lat;
    firstLon = InFhdr.proj_origin_lon;
    return 0;
  }

  if (_isEq(firstLat,InFhdr.proj_origin_lat) && _isEq(firstLon,InFhdr.proj_origin_lon))
    return 0;

  date_time_t dataTime;
  dataTime.unix_time = T;
  uconvert_from_utime( &dataTime );
  char target[1024];
  sprintf(target,"otherDir/%d%02d%02d", dataTime.year, dataTime.month, dataTime.day);

  cerr << "/bin/mkdir -p " << target << endl;
  cerr << "/bin/mv " << New.getPathInUse() << " " << target << endl << endl;

  return 0;

}

bool Process::_isEq(double a, double b){
  if (fabs(a-b) > 0.0001) return false;
  return true;
}


////////////////////////////////////////////////////
//
// Destructor
//
Process::~Process(){
  return;
}










