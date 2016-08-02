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
// TitanComplexTrack.cc
//
// Track entry object for TitanServer
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2001
//
////////////////////////////////////////////////////////////////


#include <titan/TitanComplexTrack.hh>
#include <dataport/bigend.h>
using namespace std;

////////////////////////////////////////////////////////////
// Constructor

TitanComplexTrack::TitanComplexTrack()

{

  MEM_zero(_complex_params);

}

/////////////////////////////
// Copy constructor
//

TitanComplexTrack::TitanComplexTrack(const TitanComplexTrack &rhs)
     
{
  if (this != &rhs) {
    _copy(rhs);
  }
}

////////////////////////////////////////////////////////////
// destructor

TitanComplexTrack::~TitanComplexTrack()

{
  clear();
}

/////////////////////////////
// Assignment
//

TitanComplexTrack &TitanComplexTrack::operator=(const TitanComplexTrack &rhs)
  
  
{
  return _copy(rhs);
}

//////////////////////////////////////////////////
// _copy - used by copy constructor and operator =
//

TitanComplexTrack &TitanComplexTrack::_copy(const TitanComplexTrack &rhs)

{
  
  if (&rhs == this) {
    return *this;
  }

  clear();

  _complex_params = rhs._complex_params;
  
  for (size_t ii = 0; ii < _simple_tracks.size(); ii++) {
    TitanSimpleTrack *strack = new TitanSimpleTrack(*rhs._simple_tracks[ii]);
    _simple_tracks.push_back(strack);
  }

  return *this;
  
}

////////////////////////////////////////////////////////////
// clear the object

void TitanComplexTrack::clear()

{
  
  MEM_zero(_complex_params);

  for (size_t ii = 0; ii < _simple_tracks.size(); ii++) {
    delete _simple_tracks[ii];
  }
  _simple_tracks.clear();

}


////////////////////////////////////////////////////////////
// assemble into buffer

void TitanComplexTrack::assemble(MemBuf &buf,
				 bool clear_buffer /* = false*/ ) const
  
{
  
  if (clear_buffer) {
    buf.free();
  }

  // make copy of complex_params, swap and add
  
  complex_track_params_t complex_params = _complex_params;
  complex_params.n_simple_tracks = _simple_tracks.size();
  BE_from_array_32(&complex_params, sizeof(complex_params));
  buf.add(&complex_params, sizeof(complex_params));

  // add the simple_tracks
  
  for (size_t ii = 0; ii < _simple_tracks.size(); ii++) {
    _simple_tracks[ii]->assemble(buf);
  }
  
}

////////////////////////////////////////////////////////////
// disassemble from buffer
//
// Sets len_used to the length of the buffer used while disassembling.
//
// Returns 0 on success, -1 on failure

int TitanComplexTrack::disassemble(const void *buf, int buf_len,
				   int &len_used)

{

  // clear object
  
  clear();
  
  // check min length
  
  int minLen = sizeof(_complex_params);
  if (buf_len < minLen) {
    cerr << "ERROR - TitanComplexTrack::disassemble" << endl;
    cerr << "  Buffer passed in too short" << endl;
    cerr << "  Min buffer length: " << minLen << endl;
    cerr << "  Actual buffer length: " << buf_len << endl;
    return -1;
  }

  ui08 *bptr = (ui08 *) buf;
  int lenLeft = buf_len;
  
  // copy in complex_params and swap
  
  _complex_params = *((complex_track_params_t *) bptr);
  BE_to_array_32(&_complex_params, sizeof(_complex_params));
  bptr += sizeof(_complex_params);
  lenLeft -= sizeof(_complex_params);
 
  // disassemble simple_tracks
  
  for (int i = 0; i < _complex_params.n_simple_tracks; i++) {
    TitanSimpleTrack *strack = new TitanSimpleTrack;
    int lenUsed;
    if (strack->disassemble(bptr, lenLeft, lenUsed)) {
      cerr << "ERROR - TitanComplexTrack::disassemble" << endl;
      cerr << "  Cannot disassemble simple track num: " << i << endl;
      return -1;
    }
    bptr += lenUsed;
    lenLeft -= lenUsed;
    _simple_tracks.push_back(strack);
  }
  
  // set len used

  len_used = (int) (bptr - (ui08 *) buf);

  return 0;

}

////////////////////////////////////////////////////////////
// Print

void TitanComplexTrack::print(FILE *out,
			      const storm_file_params_t &sparams,
			      const track_file_params_t &tparams)
  
{

  fprintf(out, "\n");
  fprintf(out, "    Complex track num: %d\n",
	  _complex_params.complex_track_num);
  fprintf(out, "    =================\n\n");
  
  RfPrintComplexTrackParams(out, "  ", false,
			    &tparams, &_complex_params, NULL);
  
  for (size_t ii = 0; ii < _simple_tracks.size(); ii++) {
    _simple_tracks[ii]->print(out, sparams, tparams);
  }
  
}

////////////////////////////////////////////////////////////
// PrintXML

void TitanComplexTrack::printXML(FILE *out,
			      const storm_file_params_t &sparams,
			      const track_file_params_t &tparams)
  
{

  fprintf(out, "\n");
  fprintf(out, "<complex_track complex_track_num=\"%d\">\n",
	  _complex_params.complex_track_num);
  
  RfPrintComplexTrackParamsXML(out, "  ", false,
			       &tparams, &_complex_params, NULL);
			       //&tparams, &_complex_params,  &t);
  
  for (size_t ii = 0; ii < _simple_tracks.size(); ii++) {
    _simple_tracks[ii]->printXML(out, sparams, tparams);
  }
  fprintf(out, "</complex_track>\n");
  
}

