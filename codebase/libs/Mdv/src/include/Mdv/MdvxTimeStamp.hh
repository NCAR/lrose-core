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
#ifndef _MdvxTimeStamp_hh
#define _MdvxTimeStamp_hh

#include <string>
#include <cstdio>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxChunk.hh>
#include <toolsa/DateTime.hh>
using namespace std;

//
// Forward class declarations
//

class MdvxTimeStamp
{

public:
  
  MdvxTimeStamp();
  MdvxTimeStamp(const string &name, time_t when);
  MdvxTimeStamp(const char *name, time_t when);
  MdvxTimeStamp(const MdvxChunk &chunk);
  ~MdvxTimeStamp();

  void setName( const string &newName );
  void setTime( time_t newTime );
  void setPostMark( const string &newName, time_t newTime );

  // load from a chunk
  // returns 0 on success, -1 on failure
  
  int loadFromChunk(const MdvxChunk &chunk);

  // create a chunk from the object
  // returns pointer to chunk object on success, NULL on failure

  MdvxChunk *createChunk();
  
  // data member access

  const string& getName(){ return name; }
  time_t  getTime(){ return when.utime(); }

  // printing

  void print( FILE *outfile );
  void print( ostream &out );

private:
  
  string    name;
  DateTime  when;
  
  void init();

};

#endif
