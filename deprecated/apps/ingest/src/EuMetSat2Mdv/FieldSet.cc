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
// FieldSet.cc
//
// Container for a set of defined fields.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2005
//
//////////////////////////////////////////////////////////

#include "FieldSet.hh"

using namespace std;

//////////////
// Constructor

FieldSet::FieldSet(const string &prog_name,
                   const Params &params,
                   const ChannelSet &channel_set) :
        _progName(prog_name),
        _params(params)
  
{

  _OK = true;

  // construct field objects, add to map container

  for (int ii = 0; ii < _params.fields_n; ii++) {

    Field *field = new Field(_params._fields[ii], channel_set);
    
    // check channel exists

    const Channel *channel = field->getChannel();
    if (channel == NULL) {
      cerr << "ERROR - FieldSet constructor" << endl;
      cerr << "  Field name: " << field->getName() << endl;
      cerr << "  Invalid channel id: " << field->getChannelId() << endl;
      delete field;
      _OK = false;
      return;
    }

    // check that the field units request matches the channel capability
    
    if (channel->isPassive()) {
      if (field->getOutputUnits() == Field::ALBEDO) {
        cerr << "ERROR - FieldSet constructor" << endl;
        cerr << "  Field name: " << field->getName() << endl;
        cerr << "  Channel name: " << channel->getName() << endl;
        cerr << "  This channel is passive.\n" << endl;
        cerr << "  You cannot request ALBEDO.\n" << endl;
        delete field;
        _OK = false;
        return;
      }
    } else {
      if (field->getOutputUnits() == Field::DEG_K ||
          field->getOutputUnits() == Field::DEG_C) {
        cerr << "ERROR - FieldSet constructor" << endl;
        cerr << "  Field name: " << field->getName() << endl;
        cerr << "  Channel name: " << channel->getName() << endl;
        cerr << "  This channel is not passive.\n" << endl;
        cerr << "  You cannot request DEG_K or DEG_C.\n" << endl;
        delete field;
        _OK = false;
        return;
      }
    }

    // add to containter

    FieldMapEntry entry(field->getName(), field);
    pair<FieldMap::iterator, bool> result = _fields.insert(entry);
    if (result.second == false) {
      cerr << "ERROR - FieldSet constructor" << endl;
      cerr << "  Field definition names cannot appear twice" << endl;
      cerr << "  Check field: " << field->getName() << endl;
      _OK = false;
      return;
    }

  }

}

/////////////
// Destructor

FieldSet::~FieldSet()

{

  for (FieldMap::iterator ii = _fields.begin(); ii != _fields.end(); ii++) {
    delete ii->second;
  }

}

/////////////////////////////////////////
// get a field based on the name
// returns NULL on failure

const Field *FieldSet::getField(const string &name) const

{

  FieldMap::const_iterator it = _fields.find(name);
  if (it == _fields.end()) {
    return NULL;
  } else {
    return it->second;
  }

}

