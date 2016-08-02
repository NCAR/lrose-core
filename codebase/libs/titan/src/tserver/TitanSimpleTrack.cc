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
////////////////////////////////////////////////////////////////
// TitanSimpleTrack.cc
//
// Track entry object for TitanServer
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2001
//
////////////////////////////////////////////////////////////////


#include <titan/TitanSimpleTrack.hh>
#include <dataport/bigend.h>
using namespace std;

////////////////////////////////////////////////////////////
// Constructor

TitanSimpleTrack::TitanSimpleTrack()

{

  MEM_zero(_simple_params);

}

/////////////////////////////
// Copy constructor
//

TitanSimpleTrack::TitanSimpleTrack(const TitanSimpleTrack &rhs)
     
{
  if (this != &rhs) {
    _copy(rhs);
  }
}

////////////////////////////////////////////////////////////
// destructor

TitanSimpleTrack::~TitanSimpleTrack()

{
  clear();
}

/////////////////////////////
// Assignment
//

TitanSimpleTrack &TitanSimpleTrack::operator=(const TitanSimpleTrack &rhs)
  

{
  return _copy(rhs);
}

//////////////////////////////////////////////////
// _copy - used by copy constructor and operator =
//

TitanSimpleTrack &TitanSimpleTrack::_copy(const TitanSimpleTrack &rhs)

{
  
  if (&rhs == this) {
    return *this;
  }

  clear();

  _simple_params = rhs._simple_params;
  
  for (size_t ii = 0; ii < _entries.size(); ii++) {
    TitanTrackEntry *entry = new TitanTrackEntry(*rhs._entries[ii]);
    _entries.push_back(entry);
  }

  return *this;
  
}

////////////////////////////////////////////////////////////
// clear the object

void TitanSimpleTrack::clear()

{
  
  MEM_zero(_simple_params);

  for (size_t ii = 0; ii < _entries.size(); ii++) {
    delete _entries[ii];
  }
  _entries.clear();

}


////////////////////////////////////////////////////////////
// assemble into buffer

void TitanSimpleTrack::assemble(MemBuf &buf,
			       bool clear_buffer /* = false*/ ) const

{

  if (clear_buffer) {
    buf.free();
  }

  // make copy of simple_params, swap and add
  
  simple_track_params_t simple_params = _simple_params;
  simple_params.duration_in_scans = _entries.size();
  BE_from_array_32(&simple_params, sizeof(simple_params));
  buf.add(&simple_params, sizeof(simple_params));

  // add the entries
  
  for (size_t ii = 0; ii < _entries.size(); ii++) {
    _entries[ii]->assemble(buf);
  }
  
}

////////////////////////////////////////////////////////////
// disassemble from buffer
//
// Sets len_used to the length of the buffer used while disassembling.
//
// Returns 0 on success, -1 on failure

int TitanSimpleTrack::disassemble(const void *buf, int buf_len,
				 int &len_used)

{

  // clear object

  clear();

  // check min length

  int minLen = sizeof(_simple_params);
  if (buf_len < minLen) {
    cerr << "ERROR - TitanSimpleTrack::disassemble" << endl;
    cerr << "  Buffer passed in too short" << endl;
    cerr << "  Min buffer length: " << minLen << endl;
    cerr << "  Actual buffer length: " << buf_len << endl;
    return -1;
  }

  ui08 *bptr = (ui08 *) buf;
  int lenLeft = buf_len;
  
  // copy in simple_params and swap
  
  _simple_params = *((simple_track_params_t *) bptr);
  BE_to_array_32(&_simple_params, sizeof(_simple_params));
  bptr += sizeof(_simple_params);
  lenLeft -= sizeof(_simple_params);
 
  // disassemble entries
  
  for (int i = 0; i < _simple_params.duration_in_scans; i++) {
    TitanTrackEntry *entry = new TitanTrackEntry;
    int lenUsed;
    if (entry->disassemble(bptr, lenLeft, lenUsed)) {
      cerr << "ERROR - TitanSimpleTrack::disassemble" << endl;
      cerr << "  Cannot disassemble entry num: " << i << endl;
      return -1;
    }
    bptr += lenUsed;
    lenLeft -= lenUsed;
    _entries.push_back(entry);
  }
  
  // set len used

  len_used = (int) (bptr - (ui08 *) buf);

  return 0;

}

////////////////////////////////////////////////////////////
// Print

void TitanSimpleTrack::print(FILE *out,
			     const storm_file_params_t &sparams,
			     const track_file_params_t &tparams)

{

  fprintf(out, "\n");
  fprintf(out, "    Simple track num: %d\n", _simple_params.simple_track_num);
  fprintf(out, "    ================\n\n");
  
  RfPrintSimpleTrackParams(out, "    ", &_simple_params);
  
  for (size_t ii = 0; ii < _entries.size(); ii++) {
    _entries[ii]->print(out, ii, sparams, tparams);
  }
  
}
////////////////////////////////////////////////////////////
// PrintXML

void TitanSimpleTrack::printXML(FILE *out,
			     const storm_file_params_t &sparams,
			     const track_file_params_t &tparams)

{

  fprintf(out, "\n");
  fprintf(out, "    <simple_track simple_track_num=\"%d\">\n", _simple_params.simple_track_num);
  
  RfPrintSimpleTrackParamsXML(out, "    ", &_simple_params);
  fprintf(out, "    <num_track_entries> %d </num_track_entries>\n",
          (int) _entries.size());
  for (size_t ii = 0; ii < _entries.size(); ii++) {
    _entries[ii]->printXML(out, ii, sparams, tparams);
  }
  fprintf(out, "    </simple_track>\n");
  
}

