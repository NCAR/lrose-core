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
/////////////////////////////////////////////////////////////
// FieldSet.hh
//
// Container for set of defined fields.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2005
//
/////////////////////////////////////////////////////////////

#ifndef FieldSet_HH
#define FieldSet_HH

#include <iostream>
#include <string>
#include <map>
#include "ChannelSet.hh"
#include "Field.hh"
#include "Params.hh"

using namespace std;

class FieldSet {
  
public:
  
  // constructor
  
  FieldSet(const string &prog_name,
           const Params &params,
           const ChannelSet &channel_set);
  
  // Destructor
  
  virtual ~FieldSet();
  
  // did object constuct OK?

  bool OK() { return _OK; }

  // get a field based on the name
  // returns NULL on failure

  const Field *getField(const string &name) const;

private:

  typedef map<string, Field*, less<string> > FieldMap;
  typedef pair<string, Field* > FieldMapEntry;

  bool _OK;

  const string &_progName;
  const Params &_params;

  FieldMap _fields;

};

#endif



